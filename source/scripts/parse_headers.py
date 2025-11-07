#!/usr/bin/env python3

"""
parses stk headers and generates wrapping code.

"""

import os
from pathlib import Path
# import yaml
from pprint import pprint
import dataclasses
from collections import Counter

import cxxheaderparser
from cxxheaderparser.simple import parse_file


STK = Path('build/deps/stk-install')
STK_INCLUDE = STK / 'include/stk'


class Param:
    def __init__(self, name, type, is_ref=False):
        self.name = name
        self.type = type
        self.is_ref = is_ref

    def __str__(self):
        suffix = "&" if self.is_ref else ""
        return f"{self.type}{suffix} {self.name}"

    def __repr__(self):
        return f"<Param {self}>"

    def render(self):
        suffix = "&" if self.is_ref else ""
        return f"{self.type}{suffix} {self.name}"

class MParam(Param):
    def render(self):
        suffix = "&" if self.is_ref else ""
        return f"{self.type}{suffix}"


class Method:
    def __init__(self, name, params=None, returns=None, parent=None):
        self.name = name
        self.params = params or []
        self.returns = returns
        self.parent = parent
        self.overloaded = False

    def __repr__(self):
        params = ", ".join(str(p) for p in self.params)
        return f"<Method {self.returns} {self.name}({params}) >"

    def get_type(self, p):
        suffix = "&" if p.is_ref else ""
        prefix = "stk::" if p.type.startswith('Stk') else ""
        return f"{prefix}{p.type}{suffix}"

    def render(self):
        name = self.name
        klass = self.parent.name
        if self.overloaded:
            params = ', '.join(self.get_type(p) for p in self.params)
            return f'        luabridge::overload<{params}>(&stk::{klass}::{name}),'
        return f'    .addFunction("{name}", &stk::{klass}::{name})'

    def add_param(self, name, type, is_ref=False):
        self.params.append(Param(name, type, is_ref))


class Constructor(Method):
    def __init__(self, name, params=None, parent=None):
        self.name = name
        self.params = params or []
        self.parent = parent

    def __str__(self):
        params = ", ".join(str(p) for p in self.params)
        return f"{self.name}({params})"

    def __repr__(self):
        return f"<Constructor {self} >"

    def get_type(self, p):
        suffix = "&" if p.is_ref else ""
        prefix = "stk::" if p.type.startswith('Stk') else ""
        return f"{prefix}{p.type}{suffix} {p.name}"

    def render(self):
        if self.params:
            params = ", ".join(self.get_type(p) for p in self.params)
            return f'    .addConstructor<void (*) ({params})>()'
        else:
            return f'    .addConstructor<void ()> ()'


class CppClass:
    def __init__(self, name, destructor=None, constructors=None, methods=None):
        self.name = name
        self.destructor = destructor
        self.constructors = constructors or []
        self.methods = methods or []

    def __repr__(self):
        return f"<CppClass {self.name}>"

    def get_mlist(self):
        names = set(m.name for m in self.methods if not m.name == 'tick')
        return sorted(list(names))

    def get_plist(self):
        params = set()
        for m in self.methods:
            if m.name == 'tick':
                continue
            for p in m.params:
                params.add(p.name)
        return ", ".join(params)

    def get_luafunc(self):
        name = self.name.lower()
        _name = "_" + name
        res = []
        add = res.append
        c_info = ", ".join(str(c) for c in self.constructors)
        add(f"local {_name} = stk.{self.name}() -- {c_info}")
        add(f"{name} = function(x, fb, n, {self.get_plist()})")
        for m in self.methods:
            ps = ", ".join(str(p) for p in m.params)
            add(f"    -- {_name}:{m.name}({ps})")
        add("end")
        return "\n".join(res)

    @property
    def method_counter(self):
        return Counter(m.name for m in self.methods)

    @property
    def nticks(self):
        return self.method_counter['ticks']

    def add_overloaded_method(self, name, lst):
        mcount = self.method_counter[name]
        lst.append(f'    .addFunction("{name}", ')
        methods = []
        for m in self.methods:
            if m.name == name:
                m.overloaded = True
                methods.append(m)
        for i, m in enumerate(methods):
            if i < mcount-1:
                lst.append(m.render())
            else:
                lst.append(m.render()[:-1] + ')') # last of sequence
        return lst

    def render(self):
        name = self.name
        start = f'.beginClass <stk::{name}> ("{name}")'
        end = '.endClass()'
        space = ' '*4
        res = [start]

        for c in self.constructors:
            res.append(c.render())

        cache = {}
        for m in self.methods:
            if self.method_counter[m.name] > 1 and m.name not in cache:
                res = self.add_overloaded_method(m.name, res)
                cache[m.name] = None
            elif m.name not in cache:
                res.append(m.render())

        res.append(end)
        return "\n".join(res)


def get_class(d):
    d = d['namespace']['namespaces']['stk']

    klass = CppClass(name=d['classes'][0]['class_decl']['typename']['segments'][0]['name'])

    for m in d['classes'][0]['methods']:
        if m['access'] == 'public':
            if m['constructor']:
                c = Constructor(name=m['name']['segments'][0]['name'], parent=klass)

                for p in m['parameters']:
                    name = p['name']
                    if 'typename' in p['type']:
                        typ = "::".join(s['name'] for s in p['type']['typename']['segments'])
                        c.params.append(Param(name=name, type=typ, is_ref=False))
                    elif 'ref_to' in p['type']:
                        typ = "::".join(s['name'] for s in p['type']['ref_to']['segments'])
                        c.params.append(Param(name=name, type=typ, is_ref=True))

                klass.constructors.append(c)
            elif m['destructor']:
                klass.destructor = m['name']['segments'][0]['name']
            else:
                f = Method(name = m['name']['segments'][0]['name'], parent=klass)

                if 'typename' in m['return_type']:
                    f.returns = m['return_type']['typename']['segments'][0]['name']

                for p in m['parameters']:
                    name = p['name']
                    if 'typename' in p['type']:
                        typ = "::".join(s['name'] for s in p['type']['typename']['segments'])
                        f.params.append(MParam(name=name, type=typ, is_ref=False))
                    elif 'ref_to' in p['type']:
                        typ = "::".join(s['name'] for s in p['type']['ref_to']['typename']['segments'])
                        f.params.append(MParam(name=name, type=typ, is_ref=True))

                klass.methods.append(f)

    return klass


def generate_header(classes, output_file):
    """Generate C++ header file with STK bindings for LuaBridge3"""
    header = []

    # Header guard and includes
    header.append("// GENERATED FILE - DO NOT EDIT")
    header.append("// Generated by parse_headers.py")
    header.append("// This file contains LuaBridge3 bindings for STK classes")
    header.append("")
    header.append("#pragma once")
    header.append("")
    header.append("// This macro should be called from within lstk_init_lua()")
    header.append("// after setting up the Lua state and before the try-catch block")
    header.append("#define REGISTER_STK_BINDINGS(L) \\")
    header.append("    luabridge::getGlobalNamespace(L) \\")
    header.append("    .beginNamespace(\"stk\") \\")

    # Add all class bindings
    for klass in classes:
        rendered = klass.render()
        # Indent each line and add line continuation
        lines = rendered.split('\n')
        for line in lines:
            if line.strip():  # Skip empty lines
                header.append(f"        {line} \\")

    # Close the namespace and macro
    header.append("    .endNamespace();")
    header.append("")

    # Write to file
    with open(output_file, 'w') as f:
        f.write('\n'.join(header))

    print(f"Generated {output_file} with {len(classes)} STK class bindings")


def main():
    includes = []
    classes = []
    skip = [
        'Effect',
        'FileLoop',
        'FileRead',
        'FileWrite',
        'FileWvIn',
        'FileWvOut',
        'Filter',
        'Fir',
        'FM',
        'Function',
        'Generator',
        'Iir',
        'InetWvIn',
        'InetWvOut',
        'Instrmnt',
        'Messager',
        'MidiFileIn',
        'Modal',
        'ModalBar',
        'Mutex',
        'Phonemes',
        'RtAudio',
        'RtMidi',
        'RtWvIn',
        'RtWvOut',
        'Sampler',
        'Skini',
        'SKINImsg',
        'SkiniSpec',
        'SKINItbl',
        'Socket',
        'Stk',
        'TcpClient',
        'TcpServer',
        'Thread',
        'UdpSocket',
        'Vector3D',
        'WvIn',
        'WvOut',

    ]
    for f in sorted(STK_INCLUDE.iterdir()):
        name = f.stem
        if name in skip:
            continue
        try:
            obj = parse_file(f)
            d = dataclasses.asdict(obj)

            klass = get_class(d)

        except cxxheaderparser.errors.CxxParseError:
            print("cxxheaderparser.ERROR: ", name)
            continue
        except KeyError:
            print("KeyError: ", name)
            continue
        except IndexError:
            print("IndexError: ", name)
            continue

        includes.append(f'#include "{f.name}"')

        classes.append(klass)

    # Generate the header file
    output_path = Path(__file__).parent.parent / 'projects' / 'luajit.stk~' / 'stk_bindings.h'
    generate_header(classes, output_path)

    print(f"\nSuccessfully generated bindings for {len(classes)} STK classes:")
    for c in classes:
        print(f"  - {c.name}")

main()

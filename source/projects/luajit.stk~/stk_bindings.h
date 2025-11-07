// GENERATED FILE - DO NOT EDIT MANUALLY
// STK LuaBridge3 bindings for luajit.stk~
//
// This file was extracted from luajit.stk~.cpp to improve maintainability.
// All STK includes and bindings are consolidated here.
//
// Usage: Include this file and call register_stk_bindings(L) from lstk_init_lua()

#pragma once

// STK includes
#include "ADSR.h"
#include "Asymp.h"
#include "BandedWG.h"
#include "BeeThree.h"
#include "BiQuad.h"
#include "Blit.h"
#include "BlitSaw.h"
#include "BlitSquare.h"
#include "BlowBotl.h"
#include "BlowHole.h"
#include "BowTable.h"
#include "Bowed.h"
#include "Brass.h"
#include "Chorus.h"
#include "Clarinet.h"
#include "Cubic.h"
#include "Delay.h"
#include "DelayA.h"
#include "DelayL.h"
#include "Drummer.h"
#include "Echo.h"
#include "Envelope.h"
#include "FMVoices.h"
#include "Flute.h"
#include "FormSwep.h"
#include "FreeVerb.h"
#include "Granulate.h"
#include "Guitar.h"
#include "HevyMetl.h"
#include "JCRev.h"
#include "JetTable.h"
#include "LentPitShift.h"
#include "Mandolin.h"
#include "Mesh2D.h"
#include "Modulate.h"
#include "Moog.h"
#include "NRev.h"
#include "Noise.h"
#include "OnePole.h"
#include "OneZero.h"
#include "PRCRev.h"
#include "PercFlut.h"
#include "PitShift.h"
#include "Plucked.h"
#include "PoleZero.h"
#include "Recorder.h"
#include "ReedTable.h"
#include "Resonate.h"
#include "Rhodey.h"
#include "Saxofony.h"
#include "Shakers.h"
#include "Simple.h"
#include "SineWave.h"
#include "SingWave.h"
#include "Sitar.h"
#include "Sphere.h"
#include "StifKarp.h"
#include "TapDelay.h"
#include "TubeBell.h"
#include "Twang.h"
#include "TwoPole.h"
#include "TwoZero.h"
#include "VoicForm.h"
#include "Voicer.h"
#include "Whistle.h"
#include "Wurley.h"

#include "lua.hpp"
#include <LuaBridge.h>

// Function to register all STK bindings with a Lua state
// Call this from within lstk_init_lua() inside a try-catch block
inline void register_stk_bindings(lua_State* L) {
        luabridge::getGlobalNamespace(L)
        .beginNamespace("stk")
            .beginClass <stk::ADSR> ("ADSR")
                .addConstructor<void ()> ()
                .addFunction("keyOn", &stk::ADSR::keyOn)
                .addFunction("keyOff", &stk::ADSR::keyOff)
                .addFunction("setAttackRate", &stk::ADSR::setAttackRate)
                .addFunction("setAttackTarget", &stk::ADSR::setAttackTarget)
                .addFunction("setDecayRate", &stk::ADSR::setDecayRate)
                .addFunction("setSustainLevel", &stk::ADSR::setSustainLevel)
                .addFunction("setReleaseRate", &stk::ADSR::setReleaseRate)
                .addFunction("setAttackTime", &stk::ADSR::setAttackTime)
                .addFunction("setDecayTime", &stk::ADSR::setDecayTime)
                .addFunction("setReleaseTime", &stk::ADSR::setReleaseTime)
                .addFunction("setAllTimes", &stk::ADSR::setAllTimes)
                .addFunction("setTarget", &stk::ADSR::setTarget)
                .addFunction("getState", &stk::ADSR::getState)
                .addFunction("setValue", &stk::ADSR::setValue)
                .addFunction("lastOut", &stk::ADSR::lastOut)
                .addFunction("tick", 
                    luabridge::overload<>(&stk::ADSR::tick),
                    luabridge::overload<stk::StkFrames&, unsigned int>(&stk::ADSR::tick))
            .endClass()
            .beginClass <stk::Asymp> ("Asymp")
                .addConstructor<void ()> ()
                .addFunction("keyOn", &stk::Asymp::keyOn)
                .addFunction("keyOff", &stk::Asymp::keyOff)
                .addFunction("setTau", &stk::Asymp::setTau)
                .addFunction("setTime", &stk::Asymp::setTime)
                .addFunction("setT60", &stk::Asymp::setT60)
                .addFunction("setTarget", &stk::Asymp::setTarget)
                .addFunction("setValue", &stk::Asymp::setValue)
                .addFunction("getState", &stk::Asymp::getState)
                .addFunction("lastOut", &stk::Asymp::lastOut)
                .addFunction("tick", 
                    luabridge::overload<>(&stk::Asymp::tick),
                    luabridge::overload<stk::StkFrames&, unsigned int>(&stk::Asymp::tick))
            .endClass()
            .beginClass <stk::BandedWG> ("BandedWG")
                .addConstructor<void ()> ()
                .addFunction("clear", &stk::BandedWG::clear)
                .addFunction("setStrikePosition", &stk::BandedWG::setStrikePosition)
                .addFunction("setPreset", &stk::BandedWG::setPreset)
                .addFunction("setFrequency", &stk::BandedWG::setFrequency)
                .addFunction("startBowing", &stk::BandedWG::startBowing)
                .addFunction("stopBowing", &stk::BandedWG::stopBowing)
                .addFunction("pluck", &stk::BandedWG::pluck)
                .addFunction("noteOn", &stk::BandedWG::noteOn)
                .addFunction("noteOff", &stk::BandedWG::noteOff)
                .addFunction("controlChange", &stk::BandedWG::controlChange)
                .addFunction("tick", 
                    luabridge::overload<unsigned int>(&stk::BandedWG::tick),
                    luabridge::overload<stk::StkFrames&, unsigned int>(&stk::BandedWG::tick))
            .endClass()
            .beginClass <stk::BeeThree> ("BeeThree")
                .addConstructor<void ()> ()
                .addFunction("noteOn", &stk::BeeThree::noteOn)
                .addFunction("tick", 
                    luabridge::overload<unsigned int>(&stk::BeeThree::tick),
                    luabridge::overload<stk::StkFrames&, unsigned int>(&stk::BeeThree::tick))
            .endClass()
            .beginClass <stk::BiQuad> ("BiQuad")
                .addConstructor<void ()> ()
                .addFunction("ignoreSampleRateChange", &stk::BiQuad::ignoreSampleRateChange)
                .addFunction("setCoefficients", &stk::BiQuad::setCoefficients)
                .addFunction("setB0", &stk::BiQuad::setB0)
                .addFunction("setB1", &stk::BiQuad::setB1)
                .addFunction("setB2", &stk::BiQuad::setB2)
                .addFunction("setA1", &stk::BiQuad::setA1)
                .addFunction("setA2", &stk::BiQuad::setA2)
                .addFunction("setResonance", &stk::BiQuad::setResonance)
                .addFunction("setNotch", &stk::BiQuad::setNotch)
                .addFunction("setLowPass", &stk::BiQuad::setLowPass)
                .addFunction("setHighPass", &stk::BiQuad::setHighPass)
                .addFunction("setBandPass", &stk::BiQuad::setBandPass)
                .addFunction("setBandReject", &stk::BiQuad::setBandReject)
                .addFunction("setAllPass", &stk::BiQuad::setAllPass)
                .addFunction("setEqualGainZeroes", &stk::BiQuad::setEqualGainZeroes)
                .addFunction("lastOut", &stk::BiQuad::lastOut)
                .addFunction("tick", 
                    luabridge::overload<stk::StkFloat>(&stk::BiQuad::tick),
                    luabridge::overload<stk::StkFrames&, unsigned int>(&stk::BiQuad::tick),
                    luabridge::overload<stk::StkFrames&, stk::StkFrames&, unsigned int, unsigned int>(&stk::BiQuad::tick))
            .endClass()
            .beginClass <stk::Blit> ("Blit")
                .addConstructor<void (*) (stk::StkFloat frequency)>()
                .addFunction("reset", &stk::Blit::reset)
                .addFunction("setPhase", &stk::Blit::setPhase)
                .addFunction("getPhase", &stk::Blit::getPhase)
                .addFunction("setFrequency", &stk::Blit::setFrequency)
                .addFunction("setHarmonics", &stk::Blit::setHarmonics)
                .addFunction("lastOut", &stk::Blit::lastOut)
                .addFunction("tick", 
                    luabridge::overload<>(&stk::Blit::tick),
                    luabridge::overload<stk::StkFrames&, unsigned int>(&stk::Blit::tick))
            .endClass()
            .beginClass <stk::BlitSaw> ("BlitSaw")
                .addConstructor<void (*) (stk::StkFloat frequency)>()
                .addFunction("reset", &stk::BlitSaw::reset)
                .addFunction("setFrequency", &stk::BlitSaw::setFrequency)
                .addFunction("setHarmonics", &stk::BlitSaw::setHarmonics)
                .addFunction("lastOut", &stk::BlitSaw::lastOut)
                .addFunction("tick", 
                    luabridge::overload<>(&stk::BlitSaw::tick),
                    luabridge::overload<stk::StkFrames&, unsigned int>(&stk::BlitSaw::tick))
            .endClass()
            .beginClass <stk::BlitSquare> ("BlitSquare")
                .addConstructor<void (*) (stk::StkFloat frequency)>()
                .addFunction("reset", &stk::BlitSquare::reset)
                .addFunction("setPhase", &stk::BlitSquare::setPhase)
                .addFunction("getPhase", &stk::BlitSquare::getPhase)
                .addFunction("setFrequency", &stk::BlitSquare::setFrequency)
                .addFunction("setHarmonics", &stk::BlitSquare::setHarmonics)
                .addFunction("lastOut", &stk::BlitSquare::lastOut)
                .addFunction("tick", 
                    luabridge::overload<>(&stk::BlitSquare::tick),
                    luabridge::overload<stk::StkFrames&, unsigned int>(&stk::BlitSquare::tick))
            .endClass()
            .beginClass <stk::BlowBotl> ("BlowBotl")
                .addConstructor<void ()> ()
                .addFunction("clear", &stk::BlowBotl::clear)
                .addFunction("setFrequency", &stk::BlowBotl::setFrequency)
                .addFunction("startBlowing", &stk::BlowBotl::startBlowing)
                .addFunction("stopBlowing", &stk::BlowBotl::stopBlowing)
                .addFunction("noteOn", &stk::BlowBotl::noteOn)
                .addFunction("noteOff", &stk::BlowBotl::noteOff)
                .addFunction("controlChange", &stk::BlowBotl::controlChange)
                .addFunction("tick", 
                    luabridge::overload<unsigned int>(&stk::BlowBotl::tick),
                    luabridge::overload<stk::StkFrames&, unsigned int>(&stk::BlowBotl::tick))
            .endClass()
            .beginClass <stk::BlowHole> ("BlowHole")
                .addConstructor<void (*) (stk::StkFloat lowestFrequency)>()
                .addFunction("clear", &stk::BlowHole::clear)
                .addFunction("setFrequency", &stk::BlowHole::setFrequency)
                .addFunction("setTonehole", &stk::BlowHole::setTonehole)
                .addFunction("setVent", &stk::BlowHole::setVent)
                .addFunction("startBlowing", &stk::BlowHole::startBlowing)
                .addFunction("stopBlowing", &stk::BlowHole::stopBlowing)
                .addFunction("noteOn", &stk::BlowHole::noteOn)
                .addFunction("noteOff", &stk::BlowHole::noteOff)
                .addFunction("controlChange", &stk::BlowHole::controlChange)
                .addFunction("tick", 
                    luabridge::overload<unsigned int>(&stk::BlowHole::tick),
                    luabridge::overload<stk::StkFrames&, unsigned int>(&stk::BlowHole::tick))
            .endClass()
            .beginClass <stk::BowTable> ("BowTable")
                .addConstructor<void ()> ()
                .addFunction("setOffset", &stk::BowTable::setOffset)
                .addFunction("setSlope", &stk::BowTable::setSlope)
                .addFunction("setMinOutput", &stk::BowTable::setMinOutput)
                .addFunction("setMaxOutput", &stk::BowTable::setMaxOutput)
                .addFunction("tick", 
                    luabridge::overload<stk::StkFloat>(&stk::BowTable::tick),
                    luabridge::overload<stk::StkFrames&, unsigned int>(&stk::BowTable::tick),
                    luabridge::overload<stk::StkFrames&, stk::StkFrames&, unsigned int, unsigned int>(&stk::BowTable::tick))
            .endClass()
            .beginClass <stk::Bowed> ("Bowed")
                .addConstructor<void (*) (stk::StkFloat lowestFrequency)>()
                .addFunction("clear", &stk::Bowed::clear)
                .addFunction("setFrequency", &stk::Bowed::setFrequency)
                .addFunction("setVibrato", &stk::Bowed::setVibrato)
                .addFunction("startBowing", &stk::Bowed::startBowing)
                .addFunction("stopBowing", &stk::Bowed::stopBowing)
                .addFunction("noteOn", &stk::Bowed::noteOn)
                .addFunction("noteOff", &stk::Bowed::noteOff)
                .addFunction("controlChange", &stk::Bowed::controlChange)
                .addFunction("tick", 
                    luabridge::overload<unsigned int>(&stk::Bowed::tick),
                    luabridge::overload<stk::StkFrames&, unsigned int>(&stk::Bowed::tick))
            .endClass()
            .beginClass <stk::Brass> ("Brass")
                .addConstructor<void (*) (stk::StkFloat lowestFrequency)>()
                .addFunction("clear", &stk::Brass::clear)
                .addFunction("setFrequency", &stk::Brass::setFrequency)
                .addFunction("setLip", &stk::Brass::setLip)
                .addFunction("startBlowing", &stk::Brass::startBlowing)
                .addFunction("stopBlowing", &stk::Brass::stopBlowing)
                .addFunction("noteOn", &stk::Brass::noteOn)
                .addFunction("noteOff", &stk::Brass::noteOff)
                .addFunction("controlChange", &stk::Brass::controlChange)
                .addFunction("tick", 
                    luabridge::overload<unsigned int>(&stk::Brass::tick),
                    luabridge::overload<stk::StkFrames&, unsigned int>(&stk::Brass::tick))
            .endClass()
            .beginClass <stk::Chorus> ("Chorus")
                .addConstructor<void (*) (stk::StkFloat baseDelay)>()
                .addFunction("clear", &stk::Chorus::clear)
                .addFunction("setModDepth", &stk::Chorus::setModDepth)
                .addFunction("setModFrequency", &stk::Chorus::setModFrequency)
                .addFunction("lastOut", &stk::Chorus::lastOut)
                .addFunction("tick", 
                    luabridge::overload<stk::StkFloat, unsigned int>(&stk::Chorus::tick),
                    luabridge::overload<stk::StkFrames&, unsigned int>(&stk::Chorus::tick),
                    luabridge::overload<stk::StkFrames&, stk::StkFrames&, unsigned int, unsigned int>(&stk::Chorus::tick))
            .endClass()
            .beginClass <stk::Clarinet> ("Clarinet")
                .addConstructor<void (*) (stk::StkFloat lowestFrequency)>()
                .addFunction("clear", &stk::Clarinet::clear)
                .addFunction("setFrequency", &stk::Clarinet::setFrequency)
                .addFunction("startBlowing", &stk::Clarinet::startBlowing)
                .addFunction("stopBlowing", &stk::Clarinet::stopBlowing)
                .addFunction("noteOn", &stk::Clarinet::noteOn)
                .addFunction("noteOff", &stk::Clarinet::noteOff)
                .addFunction("controlChange", &stk::Clarinet::controlChange)
                .addFunction("tick", 
                    luabridge::overload<unsigned int>(&stk::Clarinet::tick),
                    luabridge::overload<stk::StkFrames&, unsigned int>(&stk::Clarinet::tick))
            .endClass()
            .beginClass <stk::Cubic> ("Cubic")
                .addConstructor<void ()> ()
                .addFunction("setA1", &stk::Cubic::setA1)
                .addFunction("setA2", &stk::Cubic::setA2)
                .addFunction("setA3", &stk::Cubic::setA3)
                .addFunction("setGain", &stk::Cubic::setGain)
                .addFunction("setThreshold", &stk::Cubic::setThreshold)
                .addFunction("tick", 
                    luabridge::overload<stk::StkFloat>(&stk::Cubic::tick),
                    luabridge::overload<stk::StkFrames&, unsigned int>(&stk::Cubic::tick),
                    luabridge::overload<stk::StkFrames&, stk::StkFrames&, unsigned int, unsigned int>(&stk::Cubic::tick))
            .endClass()
            .beginClass <stk::Delay> ("Delay")
                .addConstructor<void (*) (unsigned long delay, unsigned long maxDelay)>()
                .addFunction("getMaximumDelay", &stk::Delay::getMaximumDelay)
                .addFunction("setMaximumDelay", &stk::Delay::setMaximumDelay)
                .addFunction("setDelay", &stk::Delay::setDelay)
                .addFunction("getDelay", &stk::Delay::getDelay)
                .addFunction("tapOut", &stk::Delay::tapOut)
                .addFunction("tapIn", &stk::Delay::tapIn)
                .addFunction("addTo", &stk::Delay::addTo)
                .addFunction("lastOut", &stk::Delay::lastOut)
                .addFunction("nextOut", &stk::Delay::nextOut)
                .addFunction("energy", &stk::Delay::energy)
                .addFunction("tick", 
                    luabridge::overload<stk::StkFloat>(&stk::Delay::tick),
                    luabridge::overload<stk::StkFrames&, unsigned int>(&stk::Delay::tick),
                    luabridge::overload<stk::StkFrames&, stk::StkFrames&, unsigned int, unsigned int>(&stk::Delay::tick))
            .endClass()
            .beginClass <stk::DelayA> ("DelayA")
                .addConstructor<void (*) (stk::StkFloat delay, unsigned long maxDelay)>()
                .addFunction("clear", &stk::DelayA::clear)
                .addFunction("getMaximumDelay", &stk::DelayA::getMaximumDelay)
                .addFunction("setMaximumDelay", &stk::DelayA::setMaximumDelay)
                .addFunction("setDelay", &stk::DelayA::setDelay)
                .addFunction("getDelay", &stk::DelayA::getDelay)
                .addFunction("tapOut", &stk::DelayA::tapOut)
                .addFunction("tapIn", &stk::DelayA::tapIn)
                .addFunction("lastOut", &stk::DelayA::lastOut)
                .addFunction("nextOut", &stk::DelayA::nextOut)
                .addFunction("tick", 
                    luabridge::overload<stk::StkFloat>(&stk::DelayA::tick),
                    luabridge::overload<stk::StkFrames&, unsigned int>(&stk::DelayA::tick),
                    luabridge::overload<stk::StkFrames&, stk::StkFrames&, unsigned int, unsigned int>(&stk::DelayA::tick))
            .endClass()
            .beginClass <stk::DelayL> ("DelayL")
                .addConstructor<void (*) (stk::StkFloat delay, unsigned long maxDelay)>()
                .addFunction("getMaximumDelay", &stk::DelayL::getMaximumDelay)
                .addFunction("setMaximumDelay", &stk::DelayL::setMaximumDelay)
                .addFunction("setDelay", &stk::DelayL::setDelay)
                .addFunction("getDelay", &stk::DelayL::getDelay)
                .addFunction("tapOut", &stk::DelayL::tapOut)
                .addFunction("tapIn", &stk::DelayL::tapIn)
                .addFunction("lastOut", &stk::DelayL::lastOut)
                .addFunction("nextOut", &stk::DelayL::nextOut)
                .addFunction("tick", 
                    luabridge::overload<stk::StkFloat>(&stk::DelayL::tick),
                    luabridge::overload<stk::StkFrames&, unsigned int>(&stk::DelayL::tick),
                    luabridge::overload<stk::StkFrames&, stk::StkFrames&, unsigned int, unsigned int>(&stk::DelayL::tick))
            .endClass()
            .beginClass <stk::Drummer> ("Drummer")
                .addConstructor<void ()> ()
                .addFunction("noteOn", &stk::Drummer::noteOn)
                .addFunction("noteOff", &stk::Drummer::noteOff)
                .addFunction("tick", 
                    luabridge::overload<unsigned int>(&stk::Drummer::tick),
                    luabridge::overload<stk::StkFrames&, unsigned int>(&stk::Drummer::tick))
            .endClass()
            .beginClass <stk::Echo> ("Echo")
                .addConstructor<void (*) (unsigned long maximumDelay)>()
                .addFunction("clear", &stk::Echo::clear)
                .addFunction("setMaximumDelay", &stk::Echo::setMaximumDelay)
                .addFunction("setDelay", &stk::Echo::setDelay)
                .addFunction("lastOut", &stk::Echo::lastOut)
                .addFunction("tick", 
                    luabridge::overload<stk::StkFloat>(&stk::Echo::tick),
                    luabridge::overload<stk::StkFrames&, unsigned int>(&stk::Echo::tick),
                    luabridge::overload<stk::StkFrames&, stk::StkFrames&, unsigned int, unsigned int>(&stk::Echo::tick))
            .endClass()
            .beginClass <stk::Envelope> ("Envelope")
                .addConstructor<void ()> ()
                .addFunction("operator=", &stk::Envelope::operator=)
                .addFunction("keyOn", &stk::Envelope::keyOn)
                .addFunction("keyOff", &stk::Envelope::keyOff)
                .addFunction("setRate", &stk::Envelope::setRate)
                .addFunction("setTime", &stk::Envelope::setTime)
                .addFunction("setTarget", &stk::Envelope::setTarget)
                .addFunction("setValue", &stk::Envelope::setValue)
                .addFunction("getState", &stk::Envelope::getState)
                .addFunction("lastOut", &stk::Envelope::lastOut)
                .addFunction("tick", 
                    luabridge::overload<>(&stk::Envelope::tick),
                    luabridge::overload<stk::StkFrames&, unsigned int>(&stk::Envelope::tick))
            .endClass()
            .beginClass <stk::FMVoices> ("FMVoices")
                .addConstructor<void ()> ()
                .addFunction("setFrequency", &stk::FMVoices::setFrequency)
                .addFunction("noteOn", &stk::FMVoices::noteOn)
                .addFunction("controlChange", &stk::FMVoices::controlChange)
                .addFunction("tick", 
                    luabridge::overload<unsigned int>(&stk::FMVoices::tick),
                    luabridge::overload<stk::StkFrames&, unsigned int>(&stk::FMVoices::tick))
            .endClass()
            .beginClass <stk::Flute> ("Flute")
                .addConstructor<void (*) (stk::StkFloat lowestFrequency)>()
                .addFunction("clear", &stk::Flute::clear)
                .addFunction("setFrequency", &stk::Flute::setFrequency)
                .addFunction("setJetReflection", &stk::Flute::setJetReflection)
                .addFunction("setEndReflection", &stk::Flute::setEndReflection)
                .addFunction("setJetDelay", &stk::Flute::setJetDelay)
                .addFunction("startBlowing", &stk::Flute::startBlowing)
                .addFunction("stopBlowing", &stk::Flute::stopBlowing)
                .addFunction("noteOn", &stk::Flute::noteOn)
                .addFunction("noteOff", &stk::Flute::noteOff)
                .addFunction("controlChange", &stk::Flute::controlChange)
                .addFunction("tick", 
                    luabridge::overload<unsigned int>(&stk::Flute::tick),
                    luabridge::overload<stk::StkFrames&, unsigned int>(&stk::Flute::tick))
            .endClass()
            .beginClass <stk::FormSwep> ("FormSwep")
                .addConstructor<void ()> ()
                .addFunction("ignoreSampleRateChange", &stk::FormSwep::ignoreSampleRateChange)
                .addFunction("setResonance", &stk::FormSwep::setResonance)
                .addFunction("setStates", &stk::FormSwep::setStates)
                .addFunction("setTargets", &stk::FormSwep::setTargets)
                .addFunction("setSweepRate", &stk::FormSwep::setSweepRate)
                .addFunction("setSweepTime", &stk::FormSwep::setSweepTime)
                .addFunction("lastOut", &stk::FormSwep::lastOut)
                .addFunction("tick", 
                    luabridge::overload<stk::StkFloat>(&stk::FormSwep::tick),
                    luabridge::overload<stk::StkFrames&, unsigned int>(&stk::FormSwep::tick),
                    luabridge::overload<stk::StkFrames&, stk::StkFrames&, unsigned int, unsigned int>(&stk::FormSwep::tick))
            .endClass()
            .beginClass <stk::FreeVerb> ("FreeVerb")
                .addConstructor<void ()> ()
                .addFunction("setEffectMix", &stk::FreeVerb::setEffectMix)
                .addFunction("setRoomSize", &stk::FreeVerb::setRoomSize)
                .addFunction("getRoomSize", &stk::FreeVerb::getRoomSize)
                .addFunction("setDamping", &stk::FreeVerb::setDamping)
                .addFunction("getDamping", &stk::FreeVerb::getDamping)
                .addFunction("setWidth", &stk::FreeVerb::setWidth)
                .addFunction("getWidth", &stk::FreeVerb::getWidth)
                .addFunction("setMode", &stk::FreeVerb::setMode)
                .addFunction("getMode", &stk::FreeVerb::getMode)
                .addFunction("clear", &stk::FreeVerb::clear)
                .addFunction("lastOut", &stk::FreeVerb::lastOut)
                .addFunction("tick", 
                    luabridge::overload<stk::StkFloat, stk::StkFloat, unsigned int>(&stk::FreeVerb::tick),
                    luabridge::overload<stk::StkFrames&, unsigned int>(&stk::FreeVerb::tick),
                    luabridge::overload<stk::StkFrames&, stk::StkFrames&, unsigned int, unsigned int>(&stk::FreeVerb::tick))
            .endClass()
            .beginClass <stk::Granulate> ("Granulate")
                .addConstructor<void ()> ()
                .addConstructor<void (*) (unsigned int nVoices, std::string fileName, bool typeRaw)>()
                .addFunction("openFile", &stk::Granulate::openFile)
                .addFunction("reset", &stk::Granulate::reset)
                .addFunction("setVoices", &stk::Granulate::setVoices)
                .addFunction("setStretch", &stk::Granulate::setStretch)
                .addFunction("setGrainParameters", &stk::Granulate::setGrainParameters)
                .addFunction("setRandomFactor", &stk::Granulate::setRandomFactor)
                .addFunction("lastOut", &stk::Granulate::lastOut)
                .addFunction("tick",
                    luabridge::overload<unsigned int>(&stk::Granulate::tick),
                    luabridge::overload<stk::StkFrames&, unsigned int>(&stk::Granulate::tick))
            .endClass()
            .beginClass <stk::Guitar> ("Guitar")
                .addConstructor<void (*) (unsigned int nStrings, std::string bodyfile)>()
                .addFunction("clear", &stk::Guitar::clear)
                .addFunction("setBodyFile", &stk::Guitar::setBodyFile)
                .addFunction("setPluckPosition", &stk::Guitar::setPluckPosition)
                .addFunction("setLoopGain", &stk::Guitar::setLoopGain)
                .addFunction("setFrequency", &stk::Guitar::setFrequency)
                .addFunction("noteOn", &stk::Guitar::noteOn)
                .addFunction("noteOff", &stk::Guitar::noteOff)
                .addFunction("controlChange", &stk::Guitar::controlChange)
                .addFunction("lastOut", &stk::Guitar::lastOut)
                .addFunction("tick", 
                    luabridge::overload<stk::StkFloat>(&stk::Guitar::tick),
                    luabridge::overload<stk::StkFrames&, unsigned int>(&stk::Guitar::tick),
                    luabridge::overload<stk::StkFrames&, stk::StkFrames&, unsigned int, unsigned int>(&stk::Guitar::tick))
            .endClass()
            .beginClass <stk::HevyMetl> ("HevyMetl")
                .addConstructor<void ()> ()
                .addFunction("noteOn", &stk::HevyMetl::noteOn)
                .addFunction("tick", 
                    luabridge::overload<unsigned int>(&stk::HevyMetl::tick),
                    luabridge::overload<stk::StkFrames&, unsigned int>(&stk::HevyMetl::tick))
            .endClass()
            .beginClass <stk::JCRev> ("JCRev")
                .addConstructor<void (*) (stk::StkFloat T60)>()
                .addFunction("clear", &stk::JCRev::clear)
                .addFunction("setT60", &stk::JCRev::setT60)
                .addFunction("lastOut", &stk::JCRev::lastOut)
                .addFunction("tick", 
                    luabridge::overload<stk::StkFloat, unsigned int>(&stk::JCRev::tick),
                    luabridge::overload<stk::StkFrames&, unsigned int>(&stk::JCRev::tick),
                    luabridge::overload<stk::StkFrames&, stk::StkFrames&, unsigned int, unsigned int>(&stk::JCRev::tick))
            .endClass()
            .beginClass <stk::JetTable> ("JetTable")
                .addFunction("tick", 
                    luabridge::overload<stk::StkFloat>(&stk::JetTable::tick),
                    luabridge::overload<stk::StkFrames&, unsigned int>(&stk::JetTable::tick),
                    luabridge::overload<stk::StkFrames&, stk::StkFrames&, unsigned int, unsigned int>(&stk::JetTable::tick))
            .endClass()
            .beginClass <stk::LentPitShift> ("LentPitShift")
                .addConstructor<void (*) (stk::StkFloat periodRatio, int tMax)>()
                .addFunction("clear", &stk::LentPitShift::clear)
                .addFunction("setShift", &stk::LentPitShift::setShift)
                .addFunction("tick", 
                    luabridge::overload<stk::StkFloat>(&stk::LentPitShift::tick),
                    luabridge::overload<stk::StkFrames&, unsigned int>(&stk::LentPitShift::tick),
                    luabridge::overload<stk::StkFrames&, stk::StkFrames&, unsigned int, unsigned int>(&stk::LentPitShift::tick))
            .endClass()
            .beginClass <stk::Mandolin> ("Mandolin")
                .addConstructor<void (*) (stk::StkFloat lowestFrequency)>()
                .addFunction("clear", &stk::Mandolin::clear)
                .addFunction("setDetune", &stk::Mandolin::setDetune)
                .addFunction("setBodySize", &stk::Mandolin::setBodySize)
                .addFunction("setPluckPosition", &stk::Mandolin::setPluckPosition)
                .addFunction("setFrequency", &stk::Mandolin::setFrequency)
                .addFunction("pluck", 
                    luabridge::overload<stk::StkFloat>(&stk::Mandolin::pluck),
                    luabridge::overload<stk::StkFloat, stk::StkFloat>(&stk::Mandolin::pluck))
                .addFunction("noteOn", &stk::Mandolin::noteOn)
                .addFunction("noteOff", &stk::Mandolin::noteOff)
                .addFunction("controlChange", &stk::Mandolin::controlChange)
                .addFunction("tick", 
                    luabridge::overload<unsigned int>(&stk::Mandolin::tick),
                    luabridge::overload<stk::StkFrames&, unsigned int>(&stk::Mandolin::tick))
            .endClass()
            .beginClass <stk::Mesh2D> ("Mesh2D")
                .addConstructor<void (*) (unsigned short nX, unsigned short nY)>()
                .addFunction("clear", &stk::Mesh2D::clear)
                .addFunction("setNX", &stk::Mesh2D::setNX)
                .addFunction("setNY", &stk::Mesh2D::setNY)
                .addFunction("setInputPosition", &stk::Mesh2D::setInputPosition)
                .addFunction("setDecay", &stk::Mesh2D::setDecay)
                .addFunction("noteOn", &stk::Mesh2D::noteOn)
                .addFunction("noteOff", &stk::Mesh2D::noteOff)
                .addFunction("energy", &stk::Mesh2D::energy)
                .addFunction("inputTick", &stk::Mesh2D::inputTick)
                .addFunction("controlChange", &stk::Mesh2D::controlChange)
                .addFunction("tick", 
                    luabridge::overload<unsigned int>(&stk::Mesh2D::tick),
                    luabridge::overload<stk::StkFrames&, unsigned int>(&stk::Mesh2D::tick))
            .endClass()
            .beginClass <stk::Modulate> ("Modulate")
                .addConstructor<void ()> ()
                .addFunction("reset", &stk::Modulate::reset)
                .addFunction("setVibratoRate", &stk::Modulate::setVibratoRate)
                .addFunction("setVibratoGain", &stk::Modulate::setVibratoGain)
                .addFunction("setRandomRate", &stk::Modulate::setRandomRate)
                .addFunction("setRandomGain", &stk::Modulate::setRandomGain)
                .addFunction("lastOut", &stk::Modulate::lastOut)
                .addFunction("tick", 
                    luabridge::overload<>(&stk::Modulate::tick),
                    luabridge::overload<stk::StkFrames&, unsigned int>(&stk::Modulate::tick))
            .endClass()
            .beginClass <stk::Moog> ("Moog")
                .addConstructor<void ()> ()
                .addFunction("setFrequency", &stk::Moog::setFrequency)
                .addFunction("noteOn", &stk::Moog::noteOn)
                .addFunction("setModulationSpeed", &stk::Moog::setModulationSpeed)
                .addFunction("setModulationDepth", &stk::Moog::setModulationDepth)
                .addFunction("controlChange", &stk::Moog::controlChange)
                .addFunction("tick", 
                    luabridge::overload<unsigned int>(&stk::Moog::tick),
                    luabridge::overload<stk::StkFrames&, unsigned int>(&stk::Moog::tick))
            .endClass()
            .beginClass <stk::NRev> ("NRev")
                .addConstructor<void (*) (stk::StkFloat T60)>()
                .addFunction("clear", &stk::NRev::clear)
                .addFunction("setT60", &stk::NRev::setT60)
                .addFunction("lastOut", &stk::NRev::lastOut)
                .addFunction("tick", 
                    luabridge::overload<stk::StkFloat, unsigned int>(&stk::NRev::tick),
                    luabridge::overload<stk::StkFrames&, unsigned int>(&stk::NRev::tick),
                    luabridge::overload<stk::StkFrames&, stk::StkFrames&, unsigned int, unsigned int>(&stk::NRev::tick))
            .endClass()
            .beginClass <stk::Noise> ("Noise")
                .addConstructor<void (*) (unsigned int seed)>()
                .addFunction("setSeed", &stk::Noise::setSeed)
                .addFunction("lastOut", &stk::Noise::lastOut)
                .addFunction("tick", 
                    luabridge::overload<>(&stk::Noise::tick),
                    luabridge::overload<stk::StkFrames&, unsigned int>(&stk::Noise::tick))
            .endClass()
            .beginClass <stk::OnePole> ("OnePole")
                .addConstructor<void (*) (stk::StkFloat thePole)>()
                .addFunction("setB0", &stk::OnePole::setB0)
                .addFunction("setA1", &stk::OnePole::setA1)
                .addFunction("setCoefficients", &stk::OnePole::setCoefficients)
                .addFunction("setPole", &stk::OnePole::setPole)
                .addFunction("lastOut", &stk::OnePole::lastOut)
                .addFunction("tick", 
                    luabridge::overload<stk::StkFloat>(&stk::OnePole::tick),
                    luabridge::overload<stk::StkFrames&, unsigned int>(&stk::OnePole::tick),
                    luabridge::overload<stk::StkFrames&, stk::StkFrames&, unsigned int, unsigned int>(&stk::OnePole::tick))
            .endClass()
            .beginClass <stk::OneZero> ("OneZero")
                .addConstructor<void (*) (stk::StkFloat theZero)>()
                .addFunction("setB0", &stk::OneZero::setB0)
                .addFunction("setB1", &stk::OneZero::setB1)
                .addFunction("setCoefficients", &stk::OneZero::setCoefficients)
                .addFunction("setZero", &stk::OneZero::setZero)
                .addFunction("lastOut", &stk::OneZero::lastOut)
                .addFunction("tick", 
                    luabridge::overload<stk::StkFloat>(&stk::OneZero::tick),
                    luabridge::overload<stk::StkFrames&, unsigned int>(&stk::OneZero::tick),
                    luabridge::overload<stk::StkFrames&, stk::StkFrames&, unsigned int, unsigned int>(&stk::OneZero::tick))
            .endClass()
            .beginClass <stk::PRCRev> ("PRCRev")
                .addConstructor<void (*) (stk::StkFloat T60)>()
                .addFunction("clear", &stk::PRCRev::clear)
                .addFunction("setT60", &stk::PRCRev::setT60)
                .addFunction("lastOut", &stk::PRCRev::lastOut)
                .addFunction("tick", 
                    luabridge::overload<stk::StkFloat, unsigned int>(&stk::PRCRev::tick),
                    luabridge::overload<stk::StkFrames&, unsigned int>(&stk::PRCRev::tick),
                    luabridge::overload<stk::StkFrames&, stk::StkFrames&, unsigned int, unsigned int>(&stk::PRCRev::tick))
            .endClass()
            .beginClass <stk::PercFlut> ("PercFlut")
                .addConstructor<void ()> ()
                .addFunction("setFrequency", &stk::PercFlut::setFrequency)
                .addFunction("noteOn", &stk::PercFlut::noteOn)
                .addFunction("tick", 
                    luabridge::overload<unsigned int>(&stk::PercFlut::tick),
                    luabridge::overload<stk::StkFrames&, unsigned int>(&stk::PercFlut::tick))
            .endClass()
            .beginClass <stk::PitShift> ("PitShift")
                .addConstructor<void ()> ()
                .addFunction("clear", &stk::PitShift::clear)
                .addFunction("setShift", &stk::PitShift::setShift)
                .addFunction("lastOut", &stk::PitShift::lastOut)
                .addFunction("tick", 
                    luabridge::overload<stk::StkFloat>(&stk::PitShift::tick),
                    luabridge::overload<stk::StkFrames&, unsigned int>(&stk::PitShift::tick),
                    luabridge::overload<stk::StkFrames&, stk::StkFrames&, unsigned int, unsigned int>(&stk::PitShift::tick))
            .endClass()
            .beginClass <stk::Plucked> ("Plucked")
                .addConstructor<void (*) (stk::StkFloat lowestFrequency)>()
                .addFunction("clear", &stk::Plucked::clear)
                .addFunction("setFrequency", &stk::Plucked::setFrequency)
                .addFunction("pluck", &stk::Plucked::pluck)
                .addFunction("noteOn", &stk::Plucked::noteOn)
                .addFunction("noteOff", &stk::Plucked::noteOff)
                .addFunction("tick", 
                    luabridge::overload<unsigned int>(&stk::Plucked::tick),
                    luabridge::overload<stk::StkFrames&, unsigned int>(&stk::Plucked::tick))
            .endClass()
            .beginClass <stk::PoleZero> ("PoleZero")
                .addConstructor<void ()> ()
                .addFunction("setB0", &stk::PoleZero::setB0)
                .addFunction("setB1", &stk::PoleZero::setB1)
                .addFunction("setA1", &stk::PoleZero::setA1)
                .addFunction("setCoefficients", &stk::PoleZero::setCoefficients)
                .addFunction("setAllpass", &stk::PoleZero::setAllpass)
                .addFunction("setBlockZero", &stk::PoleZero::setBlockZero)
                .addFunction("lastOut", &stk::PoleZero::lastOut)
                .addFunction("tick", 
                    luabridge::overload<stk::StkFloat>(&stk::PoleZero::tick),
                    luabridge::overload<stk::StkFrames&, unsigned int>(&stk::PoleZero::tick))
            .endClass()
            .beginClass <stk::Recorder> ("Recorder")
                .addConstructor<void ()> ()
                .addFunction("clear", &stk::Recorder::clear)
                .addFunction("setFrequency", &stk::Recorder::setFrequency)
                .addFunction("startBlowing", &stk::Recorder::startBlowing)
                .addFunction("stopBlowing", &stk::Recorder::stopBlowing)
                .addFunction("noteOn", &stk::Recorder::noteOn)
                .addFunction("noteOff", &stk::Recorder::noteOff)
                .addFunction("controlChange", &stk::Recorder::controlChange)
                .addFunction("tick", 
                    luabridge::overload<unsigned int>(&stk::Recorder::tick),
                    luabridge::overload<stk::StkFrames&, unsigned int>(&stk::Recorder::tick))
                .addFunction("setBlowPressure", &stk::Recorder::setBlowPressure)
                .addFunction("setVibratoGain", &stk::Recorder::setVibratoGain)
                .addFunction("setVibratoFrequency", &stk::Recorder::setVibratoFrequency)
                .addFunction("setNoiseGain", &stk::Recorder::setNoiseGain)
                .addFunction("setBreathCutoff", &stk::Recorder::setBreathCutoff)
                .addFunction("setSoftness", &stk::Recorder::setSoftness)
            .endClass()
            .beginClass <stk::ReedTable> ("ReedTable")
                .addConstructor<void ()> ()
                .addFunction("setOffset", &stk::ReedTable::setOffset)
                .addFunction("setSlope", &stk::ReedTable::setSlope)
                .addFunction("tick", 
                    luabridge::overload<stk::StkFloat>(&stk::ReedTable::tick),
                    luabridge::overload<stk::StkFrames&, unsigned int>(&stk::ReedTable::tick),
                    luabridge::overload<stk::StkFrames&, stk::StkFrames&, unsigned int, unsigned int>(&stk::ReedTable::tick))
            .endClass()
            .beginClass <stk::Resonate> ("Resonate")
                .addConstructor<void ()> ()
                .addFunction("setResonance", &stk::Resonate::setResonance)
                .addFunction("setNotch", &stk::Resonate::setNotch)
                .addFunction("setEqualGainZeroes", &stk::Resonate::setEqualGainZeroes)
                .addFunction("keyOn", &stk::Resonate::keyOn)
                .addFunction("keyOff", &stk::Resonate::keyOff)
                .addFunction("noteOn", &stk::Resonate::noteOn)
                .addFunction("noteOff", &stk::Resonate::noteOff)
                .addFunction("controlChange", &stk::Resonate::controlChange)
                .addFunction("tick", 
                    luabridge::overload<unsigned int>(&stk::Resonate::tick),
                    luabridge::overload<stk::StkFrames&, unsigned int>(&stk::Resonate::tick))
            .endClass()
            .beginClass <stk::Rhodey> ("Rhodey")
                .addConstructor<void ()> ()
                .addFunction("setFrequency", &stk::Rhodey::setFrequency)
                .addFunction("noteOn", &stk::Rhodey::noteOn)
                .addFunction("tick", 
                    luabridge::overload<unsigned int>(&stk::Rhodey::tick),
                    luabridge::overload<stk::StkFrames&, unsigned int>(&stk::Rhodey::tick))
            .endClass()
            .beginClass <stk::Saxofony> ("Saxofony")
                .addConstructor<void (*) (stk::StkFloat lowestFrequency)>()
                .addFunction("clear", &stk::Saxofony::clear)
                .addFunction("setFrequency", &stk::Saxofony::setFrequency)
                .addFunction("setBlowPosition", &stk::Saxofony::setBlowPosition)
                .addFunction("startBlowing", &stk::Saxofony::startBlowing)
                .addFunction("stopBlowing", &stk::Saxofony::stopBlowing)
                .addFunction("noteOn", &stk::Saxofony::noteOn)
                .addFunction("noteOff", &stk::Saxofony::noteOff)
                .addFunction("controlChange", &stk::Saxofony::controlChange)
                .addFunction("tick", 
                    luabridge::overload<unsigned int>(&stk::Saxofony::tick),
                    luabridge::overload<stk::StkFrames&, unsigned int>(&stk::Saxofony::tick))
            .endClass()
            .beginClass <stk::Shakers> ("Shakers")
                .addConstructor<void (*) (int type)>()
                .addFunction("noteOn", &stk::Shakers::noteOn)
                .addFunction("noteOff", &stk::Shakers::noteOff)
                .addFunction("controlChange", &stk::Shakers::controlChange)
                .addFunction("tick", 
                    luabridge::overload<unsigned int>(&stk::Shakers::tick),
                    luabridge::overload<stk::StkFrames&, unsigned int>(&stk::Shakers::tick))
            .endClass()
            .beginClass <stk::Simple> ("Simple")
                .addConstructor<void ()> ()
                .addFunction("setFrequency", &stk::Simple::setFrequency)
                .addFunction("keyOn", &stk::Simple::keyOn)
                .addFunction("keyOff", &stk::Simple::keyOff)
                .addFunction("noteOn", &stk::Simple::noteOn)
                .addFunction("noteOff", &stk::Simple::noteOff)
                .addFunction("controlChange", &stk::Simple::controlChange)
                .addFunction("tick", 
                    luabridge::overload<unsigned int>(&stk::Simple::tick),
                    luabridge::overload<stk::StkFrames&, unsigned int>(&stk::Simple::tick))
            .endClass()
            .beginClass <stk::SineWave> ("SineWave")
                .addConstructor<void ()> ()
                .addFunction("reset", &stk::SineWave::reset)
                .addFunction("setRate", &stk::SineWave::setRate)
                .addFunction("setFrequency", &stk::SineWave::setFrequency)
                .addFunction("addTime", &stk::SineWave::addTime)
                .addFunction("addPhase", &stk::SineWave::addPhase)
                .addFunction("addPhaseOffset", &stk::SineWave::addPhaseOffset)
                .addFunction("lastOut", &stk::SineWave::lastOut)
                .addFunction("tick", 
                    luabridge::overload<>(&stk::SineWave::tick),
                    luabridge::overload<stk::StkFrames&, unsigned int>(&stk::SineWave::tick))
            .endClass()
            .beginClass <stk::SingWave> ("SingWave")
                .addConstructor<void (*) (std::string fileName, bool raw)>()
                .addFunction("reset", &stk::SingWave::reset)
                .addFunction("normalize",
                    luabridge::overload<>(&stk::SingWave::normalize),
                    luabridge::overload<stk::StkFloat>(&stk::SingWave::normalize))
                .addFunction("setFrequency", &stk::SingWave::setFrequency)
                .addFunction("setVibratoRate", &stk::SingWave::setVibratoRate)
                .addFunction("setVibratoGain", &stk::SingWave::setVibratoGain)
                .addFunction("setRandomGain", &stk::SingWave::setRandomGain)
                .addFunction("setSweepRate", &stk::SingWave::setSweepRate)
                .addFunction("setGainRate", &stk::SingWave::setGainRate)
                .addFunction("setGainTarget", &stk::SingWave::setGainTarget)
                .addFunction("noteOn", &stk::SingWave::noteOn)
                .addFunction("noteOff", &stk::SingWave::noteOff)
                .addFunction("lastOut", &stk::SingWave::lastOut)
                .addFunction("tick", 
                    luabridge::overload<>(&stk::SingWave::tick),
                    luabridge::overload<stk::StkFrames&, unsigned int>(&stk::SingWave::tick))
            .endClass()
            .beginClass <stk::Sitar> ("Sitar")
                .addConstructor<void (*) (stk::StkFloat lowestFrequency)>()
                .addFunction("clear", &stk::Sitar::clear)
                .addFunction("setFrequency", &stk::Sitar::setFrequency)
                .addFunction("pluck", &stk::Sitar::pluck)
                .addFunction("noteOn", &stk::Sitar::noteOn)
                .addFunction("noteOff", &stk::Sitar::noteOff)
                .addFunction("tick", 
                    luabridge::overload<unsigned int>(&stk::Sitar::tick),
                    luabridge::overload<stk::StkFrames&, unsigned int>(&stk::Sitar::tick))
            .endClass()
            .beginClass <stk::Sphere> ("Sphere")
                .addConstructor<void (*) (stk::StkFloat radius)>()
                .addFunction("setPosition", &stk::Sphere::setPosition)
                .addFunction("setVelocity", &stk::Sphere::setVelocity)
                .addFunction("setRadius", &stk::Sphere::setRadius)
                .addFunction("setMass", &stk::Sphere::setMass)
                .addFunction("getPosition", &stk::Sphere::getPosition)
                .addFunction("getRelativePosition", &stk::Sphere::getRelativePosition)
                .addFunction("getVelocity", &stk::Sphere::getVelocity)
                .addFunction("isInside", &stk::Sphere::isInside)
                .addFunction("getRadius", &stk::Sphere::getRadius)
                .addFunction("getMass", &stk::Sphere::getMass)
                .addFunction("addVelocity", &stk::Sphere::addVelocity)
                .addFunction("tick", 
                    luabridge::overload<stk::StkFloat>(&stk::Sphere::tick))
            .endClass()
            .beginClass <stk::StifKarp> ("StifKarp")
                .addConstructor<void (*) (stk::StkFloat lowestFrequency)>()
                .addFunction("clear", &stk::StifKarp::clear)
                .addFunction("setFrequency", &stk::StifKarp::setFrequency)
                .addFunction("setStretch", &stk::StifKarp::setStretch)
                .addFunction("setPickupPosition", &stk::StifKarp::setPickupPosition)
                .addFunction("setBaseLoopGain", &stk::StifKarp::setBaseLoopGain)
                .addFunction("pluck", &stk::StifKarp::pluck)
                .addFunction("noteOn", &stk::StifKarp::noteOn)
                .addFunction("noteOff", &stk::StifKarp::noteOff)
                .addFunction("controlChange", &stk::StifKarp::controlChange)
                .addFunction("tick", 
                    luabridge::overload<unsigned int>(&stk::StifKarp::tick),
                    luabridge::overload<stk::StkFrames&, unsigned int>(&stk::StifKarp::tick))
            .endClass()

            .beginClass <stk::TapDelay> ("TapDelay")
                .addConstructor<void (*) (std::vector<unsigned long> taps, unsigned long maxDelay)>()
                .addFunction("setMaximumDelay", &stk::TapDelay::setMaximumDelay)
                .addFunction("setTapDelays", &stk::TapDelay::setTapDelays)
                .addFunction("getTapDelays", &stk::TapDelay::getTapDelays)
                .addFunction("lastOut", &stk::TapDelay::lastOut)
                .addFunction("tick", 
                    luabridge::overload<stk::StkFloat, stk::StkFrames&>(&stk::TapDelay::tick),
                    luabridge::overload<stk::StkFrames&, unsigned int>(&stk::TapDelay::tick),
                    luabridge::overload<stk::StkFrames&, stk::StkFrames&, unsigned int>(&stk::TapDelay::tick))
            .endClass()

            .beginClass <stk::TubeBell> ("TubeBell")
                .addConstructor<void ()> ()
                .addFunction("noteOn", &stk::TubeBell::noteOn)
                .addFunction("tick", 
                    luabridge::overload<unsigned int>(&stk::TubeBell::tick),
                    luabridge::overload<stk::StkFrames&, unsigned int>(&stk::TubeBell::tick))
            .endClass()
            .beginClass <stk::Twang> ("Twang")
                .addConstructor<void (*) (stk::StkFloat lowestFrequency)>()
                .addFunction("clear", &stk::Twang::clear)
                .addFunction("setLowestFrequency", &stk::Twang::setLowestFrequency)
                .addFunction("setFrequency", &stk::Twang::setFrequency)
                .addFunction("setPluckPosition", &stk::Twang::setPluckPosition)
                .addFunction("setLoopGain", &stk::Twang::setLoopGain)
                .addFunction("setLoopFilter", &stk::Twang::setLoopFilter)
                .addFunction("lastOut", &stk::Twang::lastOut)
                .addFunction("tick", 
                    luabridge::overload<stk::StkFloat>(&stk::Twang::tick),
                    luabridge::overload<stk::StkFrames&, unsigned int>(&stk::Twang::tick),
                    luabridge::overload<stk::StkFrames&, stk::StkFrames&, unsigned int, unsigned int>(&stk::Twang::tick))
            .endClass()
            .beginClass <stk::TwoPole> ("TwoPole")
                .addConstructor<void ()> ()
                .addFunction("ignoreSampleRateChange", &stk::TwoPole::ignoreSampleRateChange)
                .addFunction("setB0", &stk::TwoPole::setB0)
                .addFunction("setA1", &stk::TwoPole::setA1)
                .addFunction("setA2", &stk::TwoPole::setA2)
                .addFunction("setCoefficients", &stk::TwoPole::setCoefficients)
                .addFunction("setResonance", &stk::TwoPole::setResonance)
                .addFunction("lastOut", &stk::TwoPole::lastOut)
                .addFunction("tick", 
                    luabridge::overload<stk::StkFloat>(&stk::TwoPole::tick),
                    luabridge::overload<stk::StkFrames&, unsigned int>(&stk::TwoPole::tick),
                    luabridge::overload<stk::StkFrames&, stk::StkFrames&, unsigned int, unsigned int>(&stk::TwoPole::tick))
            .endClass()
            .beginClass <stk::TwoZero> ("TwoZero")
                .addConstructor<void ()> ()
                .addFunction("ignoreSampleRateChange", &stk::TwoZero::ignoreSampleRateChange)
                .addFunction("setB0", &stk::TwoZero::setB0)
                .addFunction("setB1", &stk::TwoZero::setB1)
                .addFunction("setB2", &stk::TwoZero::setB2)
                .addFunction("setCoefficients", &stk::TwoZero::setCoefficients)
                .addFunction("setNotch", &stk::TwoZero::setNotch)
                .addFunction("lastOut", &stk::TwoZero::lastOut)
                .addFunction("tick", 
                    luabridge::overload<stk::StkFloat>(&stk::TwoZero::tick),
                    luabridge::overload<stk::StkFrames&, unsigned int>(&stk::TwoZero::tick),
                    luabridge::overload<stk::StkFrames&, stk::StkFrames&, unsigned int, unsigned int>(&stk::TwoZero::tick))
            .endClass()
            .beginClass <stk::VoicForm> ("VoicForm")
                .addConstructor<void ()> ()
                .addFunction("clear", &stk::VoicForm::clear)
                .addFunction("setFrequency", &stk::VoicForm::setFrequency)
                .addFunction("setPhoneme", &stk::VoicForm::setPhoneme)
                .addFunction("setVoiced", &stk::VoicForm::setVoiced)
                .addFunction("setUnVoiced", &stk::VoicForm::setUnVoiced)
                .addFunction("setFilterSweepRate", &stk::VoicForm::setFilterSweepRate)
                .addFunction("setPitchSweepRate", &stk::VoicForm::setPitchSweepRate)
                .addFunction("speak", &stk::VoicForm::speak)
                .addFunction("quiet", &stk::VoicForm::quiet)
                .addFunction("noteOn", &stk::VoicForm::noteOn)
                .addFunction("noteOff", &stk::VoicForm::noteOff)
                .addFunction("controlChange", &stk::VoicForm::controlChange)
                .addFunction("tick", 
                    luabridge::overload<unsigned int>(&stk::VoicForm::tick),
                    luabridge::overload<stk::StkFrames&, unsigned int>(&stk::VoicForm::tick))
            .endClass()
            .beginClass <stk::Voicer> ("Voicer")
                .addConstructor<void (*) (stk::StkFloat decayTime)>()
                .addFunction("addInstrument", &stk::Voicer::addInstrument)
                .addFunction("removeInstrument", &stk::Voicer::removeInstrument)
                .addFunction("noteOn", &stk::Voicer::noteOn)
                .addFunction("noteOff",
                    luabridge::overload<stk::StkFloat, stk::StkFloat, int>(&stk::Voicer::noteOff),
                    luabridge::overload<long, stk::StkFloat>(&stk::Voicer::noteOff))
                .addFunction("setFrequency",
                    luabridge::overload<stk::StkFloat, int>(&stk::Voicer::setFrequency),
                    luabridge::overload<long, stk::StkFloat>(&stk::Voicer::setFrequency))

                .addFunction("pitchBend", 
                    luabridge::overload<stk::StkFloat, int>(&stk::Voicer::pitchBend),
                    luabridge::overload<long, stk::StkFloat>(&stk::Voicer::pitchBend))

                .addFunction("controlChange", 
                    luabridge::overload<int, stk::StkFloat, int>(&stk::Voicer::controlChange),
                    luabridge::overload<long, int, stk::StkFloat>(&stk::Voicer::controlChange))

                .addFunction("silence", &stk::Voicer::silence)
                .addFunction("channelsOut", &stk::Voicer::channelsOut)
                .addFunction("lastFrame", &stk::Voicer::lastFrame)
                .addFunction("lastOut", &stk::Voicer::lastOut)
                .addFunction("tick", 
                    luabridge::overload<unsigned int>(&stk::Voicer::tick),
                    luabridge::overload<stk::StkFrames&, unsigned int>(&stk::Voicer::tick))
            .endClass()
            .beginClass <stk::Whistle> ("Whistle")
                .addConstructor<void ()> ()
                .addFunction("clear", &stk::Whistle::clear)
                .addFunction("setFrequency", &stk::Whistle::setFrequency)
                .addFunction("startBlowing", &stk::Whistle::startBlowing)
                .addFunction("stopBlowing", &stk::Whistle::stopBlowing)
                .addFunction("noteOn", &stk::Whistle::noteOn)
                .addFunction("noteOff", &stk::Whistle::noteOff)
                .addFunction("controlChange", &stk::Whistle::controlChange)
                .addFunction("tick", 
                    luabridge::overload<unsigned int>(&stk::Whistle::tick),
                    luabridge::overload<stk::StkFrames&, unsigned int>(&stk::Whistle::tick))
            .endClass()
            .beginClass <stk::Wurley> ("Wurley")
                .addConstructor<void ()> ()
                .addFunction("setFrequency", &stk::Wurley::setFrequency)
                .addFunction("noteOn", &stk::Wurley::noteOn)
                .addFunction("tick", 
                    luabridge::overload<unsigned int>(&stk::Wurley::tick),
                    luabridge::overload<stk::StkFrames&, unsigned int>(&stk::Wurley::tick))
            .endClass()
        .endNamespace();
}

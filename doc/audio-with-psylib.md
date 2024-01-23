Title:Audio concepts for psylib users/developers

# Presenting audio with psylib

## Introduction

This document tries to explain how to present audio with psylib. PsyLib has
an number of instances that deal with audio. The major players are:

* [class@AudioDevice] An device that relates to to the Hardware playing/recording
  audio. In order to present AuditoryStimuli, open and start the device.
* [class@AudioMixer] The device that schedules the stimuli, to be
  presented/recorded from the AudioDevice. This device acts as an temporary
  buffer in between the audiocallback to prevent buffer under runs and overflows.
  This class is an internal class.
* [class@AuditoryStimulus] A base class that represents stimuli.
    - [class@Wave] An class derived from AuditoryStimulus it represents a
      Generated audio wave form, such as a pure tone/sine wave, white noise etc.
    - AudioFile TODO create a stimuls for presenting autitory stimuli from files

## Concepts of audio

When we are talking about sound we may talk about the samples, frames, channels
input, output etc, here we try to make a notion about which definitions that
we have in mind when talking about such an concept.

### Sample format

The sample format of the stimuli. Currently psylib only uses 32bit floating
point numbers as sample format. So a valid sample is in the range [-1.0, +1.0].
Maybe in the future extra sample formats are added.

### input vs output

**Input** is an signal that is recorded by the [class@AudioDevice]. An audio
device has an **A**nalog to **D**igital **C**onverter (ADC), this device
converts a voltage level to a digital signal. Conversely, the device also has an
**D**igital to **A**nalog **C**onverter (DAC), that converts the digital audio
into a voltage that drives an **output** device. From the perspective of psylib
we cannot know what the user has connected to the in- and output port of an
audio device, however, the inputs of an audio device are typically connected to
an microphone, or line level audio output of an MP3 player. The
output ports are typically connected to Speakers or headphones.

### Channels

An audio device has a number of input and output channels. A mono signal
comprises one channel, whereas stereo comprises 2 channels.

When recording one or multiple channels, you are able to put the multiple
channels in one audio file. When playing the same audio file, the different
channels can be played at different speakers/ears of the headphone.

In some case, psylib will duplicate mono audio to stereo audio, in such a
way, that a mono audio recording/source, will be played at both the left and
right speakers in a stereo setup, because most people will expect mono
audio to be played at both the left and right speakers of an stereo setup.

### Sample/frame Rate

The audio device can operate at different frame rates see
[enum@PsyAudioSampleRate]. CD audio has a frame rate of 44100 Hertz (Hz). this
means that for each second of audio the voltage of each channel (I think two
for CD's) is sampled 44100 times. So the frame rate and sample rate are
identical as for each audio channel one sample is take per frame.

### Audio frames vs audio samples

However, the concept of audio frame and sample are different. A sample is a
measure of the audio voltage that is sampled at the frame/sample rate of the
audio device for **one** specific channel, whereas one frame is the collection
of the samples of all channels of that frame. An analogy might be helpful here:
A frame of video comprises of all the pixels of one frame. A frame of audio
comprises all the samples for each channel. Hence, for a device opened in
stereo, with a sample rate of 44100 we sample at 44100 Hz for 2 channels.
So one second audio comprises 44100 frames, but 44100 * 2 = (88200) samples.

A sample is the smallest measurement of the audio signal at a specific point in
time, and it count for one channel. In contrast, a frame represents all samples
of that point in time, but for all the channels.

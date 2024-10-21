"""
This short program plays a Psy.Wave file. It catches the start and stop events
on the stop event the mainloop is stopped. If it doesn't stop on the mainloop,
it will exit with an error.
"""

import gi
import os
import typing
from dataclasses import dataclass

gi.require_version("Psy", "0.1")
gi.require_version("GLib", "2.0")
gi.require_version("Gst", "1.0")

# The next two line may generate warnings, ignore them ;-)
from gi.repository import Psy, GLib, Gst
import psy_operators

psy_init = Psy.Initializer()
Gst.init()


@dataclass
class WaveData:
    loop: GLib.MainLoop
    clock: Psy.Clock
    audio_dev: Psy.AudioDevice
    start: typing.Union[Psy.TimePoint, None] = None


def on_started(wave: Psy.Wave, tp: Psy.TimePoint, data: WaveData):
    """on_started is fired when psylib thinks the wave started. There might
    be - and probably is - a small discrepancy in time between the time
    this signal is the emitted and the audiocard DAC is putting the wav
    form at the speaker cable. This might be used to explore the latency
    of psylib.
    """
    data.start = data.clock.now()
    dur = data.start - wave.props.start_time
    print(f"stimulus started, start time = {dur.get_us()} µs after specified")
    assert tp == wave.props.start_time


def on_stopped(wave: Psy.Wave, tp: Psy.TimePoint, data: WaveData):
    stop = data.clock.now()
    dur = stop - wave.props.stop_time
    # data.audio_dev.close()
    print(f"stimulus stopped, stop time = {dur.get_us()} µs after specified")
    data.loop.quit()
    # assert tp == wave.props.stop_time


audio_dev = Psy.AudioDevice.new()
audio_dev.props.num_output_channels = 2
audio_dev.open()

play_dur = Psy.Duration.new_ms(500)

mainloop = GLib.MainLoop()
wavedata = WaveData(mainloop, Psy.Clock(), audio_dev)
now = wavedata.clock.now()
wav_stim = Psy.Wave.new(audio_dev)
wav_stim.props.num_channels = 1
wav_stim.props.duration = play_dur
wav_stim.props.running = True
wav_stim.connect("started", on_started, wavedata)
wav_stim.connect("stopped", on_stopped, wavedata)

wav_stim.play(now + Psy.Duration.new_ms(250))

mainloop.run()

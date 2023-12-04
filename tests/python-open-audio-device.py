import gi
import sys
import time
import argparse as ap

gi.require_versions({"Psy": "0.1", "GLib": "2.0"})

from gi.repository import Psy, GLib

g_audio_dev = None

cmdparser = ap.ArgumentParser(sys.argv[0], description="Try to open an audio device")
cmdparser.add_argument("-n", "--name", type=str, help="The name of the device")
cmdparser.add_argument(
    "-i", "--num-input", help="The number of input channels", type=int
)
cmdparser.add_argument(
    "-o", "--num-output", help="The number of output channels", type=int, default=2
)
sr_choices = [
    22050,
    24000,
    32000,
    44100,
    48000,
    88200,
    96000,
    192000,
]
cmdparser.add_argument(
    "-s",
    "--sr",
    type=int,
    help="the desired sample rate",
    choices=sr_choices,
    default=48000,
)
cmdparser.add_argument(
    "-d",
    "--duration",
    type=float,
    help="Specify a positive duration in seconds to play audio",
    default=4.0,
)


def print_audio_dev_props(dev: Psy.AudioDevice):
    print("audiodev.props.name =", dev.props.name)
    print("audiodev.props.is_open =", dev.props.is_open)
    if dev.props.is_open:
        print("audiodev.props.output_latency =", dev.props.output_latency.get_seconds())
    print("audiodev.props.started =", dev.props.started)
    print("audiodev.props.sample_rate =", dev.props.sample_rate)
    print("audiodev.props.num_input_channels =", dev.props.num_input_channels)
    print("audiodev.props.num_output_channels =", dev.props.num_output_channels)


def open_audio_device(args: ap.Namespace) -> int:
    """Opens the audio device and store it in a global variable"""
    global g_audio_dev
    g_audio_dev = Psy.AudioDevice.new()
    for i in range(3):
        print()

    if args.name:
        g_audio_dev.props.name = args.name

    if args.num_input:
        g_audio_dev.props.num_input_channels = args.num_input

    g_audio_dev.props.num_output_channels = args.num_output
    g_audio_dev.props.sample_rate = args.sr

    print("Before opening:")
    print_audio_dev_props(g_audio_dev)
    g_audio_dev.open()

    for i in range(3):
        print()
    print("After opening:")
    print_audio_dev_props(g_audio_dev)

    return GLib.SOURCE_REMOVE


def stop_loop(loop: GLib.MainLoop):
    """stops the mainloop"""
    loop.quit()


def main():
    args = cmdparser.parse_args()

    loop = GLib.MainLoop()

    GLib.idle_add(open_audio_device, args)
    GLib.timeout_add(args.duration * 1000, stop_loop, loop)

    loop.run()


if __name__ == "__main__":
    main()

import gi
import sys
import time
import argparse as ap

gi.require_versions({"Psy": "0.1", "GLib": "2.0"})

from gi.repository import Psy


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


def main():
    args = cmdparser.parse_args()
    audiodev = Psy.AudioDevice.new()
    for i in range(3):
        print()

    if args.name:
        audiodev.props.name = args.name

    if args.num_input:
        audiodev.props.num_input_channels = args.num_input

    audiodev.props.num_output_channels = args.num_output
    audiodev.props.sample_rate = args.sr

    print("Before opening:")
    print_audio_dev_props(audiodev)
    audiodev.open()

    for i in range(3):
        print()
    print("After opening:")
    print_audio_dev_props(audiodev)

    time.sleep(args.duration)


if __name__ == "__main__":
    main()

# Copyright (c) 2025 Gong Zhile <gongzl@stu.hebust.edu.cn>
#
# SPDX-License-Identifier: Apache-2.0

'''Runner for flashing with ws63flash.'''

import platform
from os import path

from runners.core import RunnerCaps, ZephyrBinaryRunner

DEFAULT_DEVICE = '/dev/ttyUSB0'
if platform.system() == 'Darwin':
    DEFAULT_DEVICE = '/dev/tty.SLAB_USBtoUART'

class Ws63flashBinaryRunner(ZephyrBinaryRunner):
    '''Runner front-end for ws63flash.'''

    def __init__(self, cfg, device, action='write', baud=921600):
        super().__init__(cfg)

        self.device = device
        self.action = action
        self.baud = baud

    @classmethod
    def name(cls):
        return 'ws63flash'

    @classmethod
    def capabilities(cls):
        return RunnerCaps(commands={'flash'}, reset=True)

    @classmethod
    def do_add_parser(cls, parser):
        parser.add_argument('--device', default=DEFAULT_DEVICE, required=False,
                            help='serial port to flash, default \'' + DEFAULT_DEVICE + '\'')
        parser.add_argument('--action', default='write', required=False,
                            choices=['erase', 'info', 'start', 'write'],
                            help='erase / get device info / start execution / write flash')
        parser.add_argument('--baud-rate', default='921600', required=False,
                            choices=['115200', '230400', '460800', '500000', '576000', '921600', '1000000', '1152000', '1500000', '2000000'],
                            help='serial baud rate, default \'921600\'')
        parser.set_defaults(reset=False)

    @classmethod
    def do_create(cls, cfg, args):
        return Ws63flashBinaryRunner(cfg, device=args.device, action=args.action,
                                     baud=args.baud_rate)

    def do_run(self, command, **kwargs):
        self.require('ws63flash')
        self.ensure_output('bin')

        bin_name = self.cfg.bin_file
        bin_size = path.getsize(bin_name)

        cmd_flash = ['ws63flash', '-b', self.baud]

        action = self.action.lower()

        if action == 'erase':
            cmd_flash.extend(['--erase', self.device])

        elif action == 'write':
            msg_text = f"write {bin_size} bytes"
            cmd_flash.extend(['--write-program', self.device, bin_name])

        else:
            msg_text = f"invalid action \'{action}\' passed!"
            self.logger.error(f'Invalid action \'{action}\' passed!')
            return -1

        self.logger.info("Board: " + msg_text)
        self.check_call(cmd_flash)
        self.logger.info(f'Board: finished \'{action}\' .')


# PS2 Joypad Server

Use PlayStation 2 input accessories that cheap "PS2 to USB" dongles don't recognize by dumping gamepad input into UDP packets.

## Building from source

(Tested only on Linux)

0. Set up [PS2SDK](https://github.com/ps2dev/ps2dev)
1. Run `make`

## Configuration

- Set up OPL networking within OPL and save;
- Copy the ELF to somewhere accessible (e.g.: MC1:/APPS/JOYSRV.ELF);
- Configure FMCB shortcuts.

## The client

There must be a UDP listener at:
- Same IP as configured in OPL;
- The port for SMB in OPL plus 1024:
    - If you used the default port 445, it'll direct UDP towards port 1469.
- Capable of parsing and creating input in the system.


### Known ready-to-use implementations

 - [ps2joycln](https://github.com/adlerosn/ps2joycln)

### Data format

Expect 71 bytes per datagram. The last byte will always be a '`\n`'. Therefore looking like:

`1006200079FFFF8080808000000000000000000000000000000000000000000000005A` - DualShock 2

`2002200041FFFF0000000000000000000000000000000000000000000000000000005A` - 2011 generic Dance Mat

After converting the trimmed hex string into a ByteArray:

- 1st byte: where is the controller connected
    - 1st nibble: Controller connected to port 1 or 2
    - 2nd nibble: [unused] The slot of the multitap that the gamepad is connected to
- 2nd byte: Pad state. Usable if either 2 or 6; unusable otherwise ([documentation on PAD_STATE_*](https://ps2dev.github.io/ps2sdk/libpad_8h.html))
- 3rd byte: Pad read status. Usable if non-zero; unusable otherwise ([documentation on padRead()](https://ps2dev.github.io/ps2sdk/libpad_8h.html#a3ce97ac47e0c081994494f37280c0bcb))
- 4th to 35th byte: [struct pad_button_status](https://ps2dev.github.io/ps2sdk/libpad_8h.html#structpad_button_status)
    -  4th byte: `u8 ok` - zero is good 
    -  5th byte: `u8 mode` - ???
    -  6th and 7th bytes: `u16le btns` - bit array for digital input for buttons; 1 = released, 0 = pressed.
        - The field itself stores little endian, but here I show big endian for clarity:
        - `0x0001` - `0b0000000000000001` - PAD_SELECT
        - `0x0002` - `0b0000000000000010` - PAD_L3
        - `0x0004` - `0b0000000000000100` - PAD_R3
        - `0x0008` - `0b0000000000001000` - PAD_START
        - `0x0010` - `0b0000000000010000` - PAD_UP
        - `0x0020` - `0b0000000000100000` - PAD_RIGHT
        - `0x0040` - `0b0000000001000000` - PAD_DOWN
        - `0x0080` - `0b0000000010000000` - PAD_LEFT
        - `0x0100` - `0b0000000100000000` - PAD_L2
        - `0x0200` - `0b0000001000000000` - PAD_R2
        - `0x0400` - `0b0000010000000000` - PAD_L1
        - `0x0800` - `0b0000100000000000` - PAD_R1
        - `0x1000` - `0b0001000000000000` - PAD_TRIANGLE
        - `0x2000` - `0b0010000000000000` - PAD_CIRCLE
        - `0x4000` - `0b0100000000000000` - PAD_CROSS
        - `0x8000` - `0b1000000000000000` - PAD_SQUARE
    - 8th byte: `u8 rjoy_h` - `00=left, FF=right, 80=neutral` - Right analog stick
    - 9th byte: `u8 rjoy_v` - `00=up,__ FF=down,_ 80=neutral` - Right analog stick
    - 10th byte: `u8 ljoy_h` - `00=left, FF=right, 80=neutral` - Left analog stick
    - 11th byte: `u8 ljoy_v` - `00=up,__ FF=down,_ 80=neutral` - Left analog stick
    - 12th byte: `u8 right_p` - Pressure senstivity for right directional - `00=released, FF=full pressure`
    - 13th byte: `u8 left_p` - Pressure senstivity for left directional - `00=released, FF=full pressure`
    - 14th byte: `u8 up_p` - Pressure senstivity for up directional - `00=released, FF=full pressure`
    - 15th byte: `u8 down_p` - Pressure senstivity for down directional - `00=released, FF=full pressure`
    - 16th byte: `u8 triangle_p` - Pressure senstivity for triangle - `00=released, FF=full pressure`
    - 17th byte: `u8 circle_p` - Pressure senstivity for circle - `00=released, FF=full pressure`
    - 18th byte: `u8 cross_p` - Pressure senstivity for cross - `00=released, FF=full pressure`
    - 19th byte: `u8 square_p` - Pressure senstivity for circle - `00=released, FF=full pressure`
    - 20th byte: `u8 l1_p` - Pressure senstivity for L1 - `00=released, FF=full pressure`
    - 21th byte: `u8 r1_p` - Pressure senstivity for R1 - `00=released, FF=full pressure`
    - 22th byte: `u8 l2_p` - Pressure senstivity for L2 - `00=released, FF=full pressure`
    - 23th byte: `u8 r2_p` - Pressure senstivity for R2 - `00=released, FF=full pressure`
    - 24th to 35th byte - ???

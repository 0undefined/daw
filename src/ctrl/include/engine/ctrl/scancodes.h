#ifndef ENGINE_CTRL_SCANCODES_H

// We want to reserve the following bytes marked with X for MODS,
// one bit per MODIFIER (shift, control, alt, super):
// XXXX 0000 0000 0000  0000 0000 0000 0000
//
// We want to reserve the following bytes marked with X for ACTIONS
// enumerate by action type, pressed, released, repeat
// 0000 0000 XXXX 0000  0000 0000 0000 0000
//
// We want to reserve the following bytes marked with X for the SCANCODE
// 0000 0000 0000 0000  XXXX XXXX XXXX XXXX
#define MOD(x) 1 << (32-4 + x)
#define ACTION(x) (1 << (32-8)) + x

typedef enum {
  KEY_SPACE         = ' ',
  KEY_APOSTROPHE    = '\'',
  KEY_COMMA         = ',',
  KEY_MINUS         = '-',
  KEY_PERIOD        = '.',
  KEY_SLASH         = '/',

  KEY_0             = '0',
  KEY_1             = '1',
  KEY_2             = '2',
  KEY_3             = '3',
  KEY_4             = '4',
  KEY_5             = '5',
  KEY_6             = '6',
  KEY_7             = '7',
  KEY_8             = '8',
  KEY_9             = '9',

  KEY_SEMICOLON     = ';',
  KEY_EQUAL         = '=',

  KEY_A             = 'A',
  KEY_B             = 'B',
  KEY_C             = 'C',
  KEY_D             = 'D',
  KEY_E             = 'E',
  KEY_F             = 'F',
  KEY_G             = 'G',
  KEY_H             = 'H',
  KEY_I             = 'I',
  KEY_J             = 'J',
  KEY_K             = 'K',
  KEY_L             = 'L',
  KEY_M             = 'M',
  KEY_N             = 'N',
  KEY_O             = 'O',
  KEY_P             = 'P',
  KEY_Q             = 'Q',
  KEY_R             = 'R',
  KEY_S             = 'S',
  KEY_T             = 'T',
  KEY_U             = 'U',
  KEY_V             = 'V',
  KEY_W             = 'W',
  KEY_X             = 'X',
  KEY_Y             = 'Y',
  KEY_Z             = 'Z',

  KEY_LEFT_BRACKET  = '[',
  KEY_BACKSLASH     = '\\',
  KEY_RIGHT_BRACKET = ']',
  KEY_GRAVE_ACCENT  = '`',

  ACTION_PRESS      = ACTION(0),
  ACTION_RELEASE    = ACTION(1),
  ACTION_REPEAT     = ACTION(2),

  MOD_SHIFT         = MOD(0),
  MOD_CONTROL       = MOD(1),
  MOD_ALT           = MOD(2),
  MOD_SUPER         = MOD(3),
} ScanCode;
#undef ACTION
#undef MOD

#endif

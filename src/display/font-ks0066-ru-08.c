#include <avr/pgmspace.h>

const uint8_t font_ks0066_ru_08[] PROGMEM = {

    0x01, // font height in bytes
    0x7F, // position of symbol-space between letters
    0xC0, // count of chars
    0x20, // ascii char offset
    0x40, // non-ascii char offset

    // char widths
    0x05, 0x01, 0x03, 0x05, 0x05, 0x05, 0x05, 0x02,
    0x03, 0x03, 0x05, 0x05, 0x02, 0x05, 0x01, 0x05,
    0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
    0x05, 0x05, 0x01, 0x02, 0x04, 0x05, 0x04, 0x05,
    0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
    0x05, 0x03, 0x05, 0x05, 0x04, 0x05, 0x05, 0x05,
    0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
    0x05, 0x05, 0x05, 0x03, 0x05, 0x03, 0x05, 0x05,
    0x02, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
    0x05, 0x03, 0x04, 0x04, 0x03, 0x05, 0x04, 0x05,
    0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
    0x05, 0x05, 0x05, 0x03, 0x01, 0x03, 0x05, 0x01,

    0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
    0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
    0x05, 0x05, 0x05, 0x04, 0x04, 0x05, 0x05, 0x04,
    0x05, 0x05, 0x04, 0x04, 0x05, 0x04, 0x04, 0x05,
    0x04, 0x05, 0x05, 0x05, 0x04, 0x05, 0x05, 0x05,
    0x05, 0x05, 0x05, 0x05, 0x00, 0x05, 0x05, 0x05,
    0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x03, 0x03,
    0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
    0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
    0x03, 0x05, 0x05, 0x04, 0x03, 0x05, 0x05, 0x04,
    0x05, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05,

    // font data
    // 32  space
    0x00, 0x00, 0x00, 0x00, 0x00,
    // 33  !
    0x5F,
    // 34  "
    0x07, 0x00, 0x07,
    // 35  #
    0x14, 0x7F, 0x14, 0x7F, 0x14,
    // 36  $
    0x24, 0x2A, 0x7F, 0x2A, 0x12,
    // 37  %
    0x23, 0x13, 0x08, 0x64, 0x62,
    // 38  &
    0x36, 0x49, 0x55, 0x22, 0x50,
    // 39  '
    0x05, 0x03,
    // 40  (
    0x1C, 0x22, 0x41,
    // 41  )
    0x41, 0x22, 0x1C,
    // 42  *
    0x14, 0x08, 0x3E, 0x08, 0x14,
    // 43  +
    0x08, 0x08, 0x3E, 0x08, 0x08,
    // 44  ,
    0x80, 0x40,
    // 45  -
    0x08, 0x08, 0x08, 0x08, 0x08,
    // 46  .
    0x40,
    // 47  /
    0x20, 0x10, 0x08, 0x04, 0x02,
    // 48  0
    0x3E, 0x51, 0x49, 0x45, 0x3E,
    // 49  1
    0x00, 0x42, 0x7F, 0x40, 0x00,
    // 50  2
    0x42, 0x61, 0x51, 0x49, 0x46,
    // 51  3
    0x21, 0x41, 0x45, 0x4B, 0x31,
    // 52  4
    0x18, 0x14, 0x12, 0x7F, 0x10,
    // 53  5
    0x27, 0x45, 0x45, 0x45, 0x39,
    // 54  6
    0x3C, 0x4A, 0x49, 0x49, 0x30,
    // 55  7
    0x01, 0x71, 0x09, 0x05, 0x03,
    // 56  8
    0x36, 0x49, 0x49, 0x49, 0x36,
    // 57  9
    0x06, 0x49, 0x49, 0x29, 0x1E,
    // 58  :
    0x24,
    // 59  ;
    0x40, 0x24,
    // 60  <
    0x08, 0x14, 0x22, 0x41,
    // 61  =
    0x14, 0x14, 0x14, 0x14, 0x14,
    // 62  >
    0x41, 0x22, 0x14, 0x08,
    // 63  ?
    0x02, 0x01, 0x51, 0x09, 0x06,
    // 64  @
    0x32, 0x49, 0x79, 0x41, 0x3E,
    // 65  A
    0x7E, 0x11, 0x11, 0x11, 0x7E,
    // 66  B
    0x7F, 0x49, 0x49, 0x49, 0x36,
    // 67  C
    0x3E, 0x41, 0x41, 0x41, 0x22,
    // 68  D
    0x7F, 0x41, 0x41, 0x22, 0x1C,
    // 69  E
    0x7F, 0x49, 0x49, 0x49, 0x41,
    // 70  F
    0x7F, 0x09, 0x09, 0x09, 0x01,
    // 71  G
    0x3E, 0x41, 0x49, 0x49, 0x7A,
    // 72  H
    0x7F, 0x08, 0x08, 0x08, 0x7F,
    // 73  I
    0x41, 0x7F, 0x41,
    // 74  J
    0x20, 0x40, 0x41, 0x3F, 0x01,
    // 75  K
    0x7F, 0x08, 0x14, 0x22, 0x41,
    // 76  L
    0x7F, 0x40, 0x40, 0x40,
    // 77  M
    0x7F, 0x02, 0x0C, 0x02, 0x7F,
    // 78  N
    0x7F, 0x04, 0x08, 0x10, 0x7F,
    // 79  O
    0x3E, 0x41, 0x41, 0x41, 0x3E,
    // 80  P
    0x7F, 0x09, 0x09, 0x09, 0x06,
    // 81  Q
    0x3E, 0x41, 0x51, 0x61, 0x3E,
    // 82  R
    0x7F, 0x09, 0x19, 0x29, 0x46,
    // 83  S
    0x26, 0x49, 0x49, 0x49, 0x32,
    // 84  T
    0x01, 0x01, 0x7F, 0x01, 0x01,
    // 85  U
    0x3F, 0x40, 0x40, 0x40, 0x3F,
    // 86  V
    0x07, 0x18, 0x60, 0x18, 0x07,
    // 87  W
    0x3F, 0x40, 0x3C, 0x40, 0x3F,
    // 88  X
    0x63, 0x14, 0x08, 0x14, 0x63,
    // 89  Y
    0x07, 0x08, 0x70, 0x08, 0x07,
    // 90  Z
    0x61, 0x51, 0x49, 0x45, 0x43,
    // 91  [
    0x7F, 0x41, 0x41,
    // 92
    0x02, 0x04, 0x08, 0x10, 0x20,
    // 93  ]
    0x41, 0x41, 0x7F,
    // 94  ^
    0x04, 0x02, 0x01, 0x02, 0x04,
    // 95  _
    0x40, 0x40, 0x40, 0x40, 0x40,
    // 96  `
    0x01, 0x02,
    // 97  a
    0x20, 0x54, 0x54, 0x54, 0x78,
    // 98  b
    0x7F, 0x48, 0x44, 0x44, 0x38,
    // 99  c
    0x38, 0x44, 0x44, 0x44, 0x20,
    // 100  d
    0x38, 0x44, 0x44, 0x48, 0x7F,
    // 101  e
    0x38, 0x54, 0x54, 0x54, 0x18,
    // 102  f
    0x08, 0x7E, 0x09, 0x01, 0x02,
    // 103  g
    0x18, 0xA4, 0xA4, 0xA4, 0x7C,
    // 104  h
    0x7F, 0x08, 0x04, 0x04, 0x78,
    // 105  i
    0x44, 0x7D, 0x40,
    // 106  j
    0x40, 0x80, 0x84, 0x7D,
    // 107  k
    0x7F, 0x10, 0x28, 0x44,
    // 108  l
    0x41, 0x7F, 0x40,
    // 109  m
    0x7C, 0x04, 0x18, 0x04, 0x78,
    // 110  n
    0x7C, 0x08, 0x04, 0x78,
    // 111  o
    0x38, 0x44, 0x44, 0x44, 0x38,
    // 112  p
    0xFC, 0x24, 0x44, 0x44, 0x38,
    // 113  q
    0x38, 0x44, 0x44, 0x24, 0xFC,
    // 114  r
    0x7C, 0x08, 0x04, 0x04, 0x08,
    // 115  s
    0x48, 0x54, 0x54, 0x54, 0x20,
    // 116  t
    0x04, 0x3F, 0x44, 0x40, 0x20,
    // 117  u
    0x3C, 0x40, 0x40, 0x20, 0x7C,
    // 118  v
    0x0C, 0x30, 0x40, 0x30, 0x0C,
    // 119  w
    0x3C, 0x40, 0x30, 0x40, 0x3C,
    // 120  x
    0x44, 0x28, 0x10, 0x28, 0x44,
    // 121  y
    0x0C, 0x50, 0x50, 0x50, 0x3C,
    // 122  z
    0x44, 0x64, 0x54, 0x4C, 0x44,
    // 123  {
    0x08, 0x36, 0x41,
    // 124  |
    0x7F,
    // 125  }
    0x41, 0x36, 0x08,
    // 126  ~
    0x08, 0x04, 0x08, 0x08, 0x04,
    // 127  ltspace
    0x00,
    // 160  Б
    0x7F, 0x49, 0x49, 0x49, 0x31,
    // 161  Г
    0x7F, 0x01, 0x01, 0x01, 0x03,
    // 162  Ё
    0x7E, 0x4B, 0x4A, 0x4B, 0x42,
    // 163  Ж
    0x77, 0x08, 0x7F, 0x08, 0x77,
    // 164  З
    0x41, 0x49, 0x49, 0x49, 0x36,
    // 165  И
    0x7F, 0x10, 0x08, 0x04, 0x7F,
    // 166  Й
    0x7E, 0x20, 0x13, 0x08, 0x7E,
    // 167  Л
    0x40, 0x3C, 0x02, 0x01, 0x7F,
    // 168  П
    0x7F, 0x01, 0x01, 0x01, 0x7F,
    // 169  У
    0x07, 0x48, 0x48, 0x48, 0x3F,
    // 170  Ф
    0x1C, 0x22, 0x7F, 0x22, 0x1C,
    // 171  Ч
    0x07, 0x08, 0x08, 0x08, 0x7F,
    // 172  Ш
    0x7F, 0x40, 0x7F, 0x40, 0x7F,
    // 173  Ъ
    0x01, 0x7F, 0x48, 0x48, 0x30,
    // 174  Ы
    0x7F, 0x48, 0x30, 0x00, 0x7F,
    // 175  Э
    0x22, 0x41, 0x49, 0x49, 0x3E,
    // 176  Ю
    0x7F, 0x08, 0x3E, 0x41, 0x3E,
    // 177  Я
    0x46, 0x29, 0x19, 0x09, 0x7F,
    // 178  б
    0x3C, 0x4A, 0x4A, 0x49, 0x30,
    // 179  в
    0x7C, 0x54, 0x54, 0x28,
    // 180  г
    0x7C, 0x04, 0x04, 0x0C,
    // 181  ё
    0x38, 0x55, 0x54, 0x55, 0x18,
    // 182  ж
    0x6C, 0x10, 0x7C, 0x10, 0x6C,
    // 183  з
    0x44, 0x54, 0x54, 0x28,
    // 184  и
    0x7C, 0x20, 0x10, 0x08, 0x7C,
    // 185  й
    0x7C, 0x21, 0x12, 0x09, 0x7C,
    // 186  к
    0x7C, 0x10, 0x28, 0x44,
    // 187  л
    0x70, 0x08, 0x04, 0x7C,
    // 188  м
    0x7C, 0x08, 0x10, 0x08, 0x7C,
    // 189  н
    0x7C, 0x10, 0x10, 0x7C,
    // 190  п
    0x7C, 0x04, 0x04, 0x7C,
    // 191  т
    0x04, 0x04, 0x7C, 0x04, 0x04,
    // 192  ч
    0x0C, 0x10, 0x10, 0x7C,
    // 193  ш
    0x7C, 0x40, 0x7C, 0x40, 0x7C,
    // 194  ъ
    0x04, 0x7C, 0x50, 0x50, 0x20,
    // 195  ы
    0x7C, 0x50, 0x20, 0x00, 0x7C,
    // 196  ь
    0x7C, 0x50, 0x50, 0x20,
    // 197  э
    0x28, 0x44, 0x54, 0x54, 0x38,
    // 198  ю
    0x7C, 0x10, 0x38, 0x44, 0x38,
    // 199  я
    0x08, 0x54, 0x34, 0x14, 0x7C,
    // 200  Â
    0x7C, 0x12, 0x13, 0x12, 0x7C,
    // 201  â
    0x20, 0x54, 0x55, 0x54, 0x78,
    // 202  À
    0x7C, 0x13, 0x12, 0x12, 0x7C,
    // 203  à
    0x20, 0x55, 0x54, 0x54, 0x78,
    // 205  ¿
    0x30, 0x48, 0x45, 0x40, 0x20,
    // 206  Ÿ
    0x06, 0x09, 0x70, 0x09, 0x06,
    // 207  ÿ
    0x0C, 0x51, 0x50, 0x51, 0x3C,
    // 208  É
    0x7E, 0x4A, 0x4A, 0x4B, 0x42,
    // 209  é
    0x38, 0x54, 0x54, 0x55, 0x18,
    // 210  Ê
    0x7E, 0x4A, 0x4B, 0x4A, 0x42,
    // 211  ê
    0x38, 0x54, 0x55, 0x54, 0x18,
    // 212  È
    0x7E, 0x4B, 0x4A, 0x4A, 0x42,
    // 213  è
    0x38, 0x55, 0x54, 0x54, 0x18,
    // 214  Î
    0x42, 0x7F, 0x42,
    // 215  î
    0x44, 0x7D, 0x40,
    // 216  Ô
    0x3C, 0x42, 0x43, 0x42, 0x3C,
    // 217  ô
    0x38, 0x44, 0x45, 0x44, 0x38,
    // 218  Û
    0x3E, 0x40, 0x41, 0x40, 0x3E,
    // 219  û
    0x3C, 0x40, 0x41, 0x20, 0x7C,
    // 220  Ù
    0x3E, 0x41, 0x40, 0x40, 0x3E,
    // 221  ù
    0x3C, 0x41, 0x40, 0x20, 0x7C,
    // 222  Ü
    0x3E, 0x41, 0x40, 0x41, 0x3E,
    // 223  ü
    0x3C, 0x41, 0x40, 0x21, 0x7C,
    // 224  Д
    0xC0, 0x7E, 0x41, 0x41, 0xFF,
    // 225  Ц
    0x7F, 0x40, 0x40, 0x7F, 0xC0,
    // 226  Щ
    0x7F, 0x40, 0x7F, 0x40, 0xFF,
    // 227  д
    0xC0, 0x78, 0x44, 0x7C, 0xC0,
    // 228  ф
    0x30, 0x48, 0xFC, 0x48, 0x30,
    // 229  ц
    0x7C, 0x40, 0x40, 0x7C, 0xC0,
    // 230  щ
    0x7C, 0x40, 0x7C, 0x40, 0xFC,
    // 231  Ґ
    0x7E, 0x02, 0x02, 0x02, 0x03,
    // 232  Ї
    0x43, 0x7E, 0x43,
    // 233  Є
    0x3E, 0x49, 0x49, 0x41, 0x22,
    // 234  Ў
    0x06, 0x49, 0x4A, 0x49, 0x3E,
    // 235  ґ
    0x7C, 0x04, 0x04, 0x06,
    // 236  ї
    0x45, 0x7C, 0x41,
    // 237  є
    0x38, 0x54, 0x54, 0x44, 0x28,
    // 238  ў
    0x0C, 0x51, 0x52, 0x51, 0x3C,
    // 239  °
    0x06, 0x09, 0x09, 0x06,
    // 240  Ç
    0x1E, 0x21, 0x61, 0x21, 0x12,
    // 241  ç
    0x18, 0x24, 0x64, 0x24, 0x10,
    // 255  fill
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF,

};
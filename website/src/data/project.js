export const project = Object.freeze({
  name: 'Corney',
  repository: 'https://github.com/maxistar/corney',
  helper: 'https://projects.maxistar.me/keyboard_helper/',
  helperRepository: 'https://github.com/maxistar/keyboard_helper',
  printable: Object.freeze({
    choc: 'https://github.com/maxistar/corney/tree/main/body/Choc_Version',
    mx: 'https://github.com/maxistar/corney/tree/main/body/MX_Version',
  }),
  docs: Object.freeze({
    oled: 'https://github.com/maxistar/corney/blob/main/docs/dongle-hybrid-status-screen-verification.md',
    pointing: 'https://github.com/maxistar/corney/blob/main/docs/dual-side-trackpad-verification.md',
    protocol: 'https://github.com/maxistar/corney/blob/main/docs/keyboard-helper-ble-v1.md',
    readme: 'https://github.com/maxistar/corney/blob/main/readme.md',
    keymap: 'https://github.com/maxistar/corney/blob/main/config/boards/shields/corney/corney.keymap',
  }),
});

export const hardware = Object.freeze({
  controller: 'nice!nano v2',
  display: '128 × 64 SSD1306, I²C address 0x3c, 3.3 V',
  trackpad: 'Optional Cirque Pinnacle on either half',
});

export const photos = Object.freeze({
  choc: Object.freeze({
    src: '/corney/images/corney-choc.webp',
    alt: 'Two assembled black low-profile Corney halves on a wooden desk.',
  }),
  mx: Object.freeze({
    src: '/corney/images/corney-mx.webp',
    alt: 'Two assembled Corney halves with white MX keycaps viewed from above.',
  }),
});

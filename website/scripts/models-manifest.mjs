// Published STL models: `target` is the path under public/models/, `source` is the file under body/.
// Edit this list when adding or removing a model; scripts/sync-models.mjs copies it into place.
export const modelManifest = Object.freeze([
  ['choc/board_cover.stl', 'Choc_Version/board_cover.stl'],
  ['choc/board_cover_reset_button.stl', 'Choc_Version/board_cover_reset_button.stl'],
  ['choc/body_thin.stl', 'Choc_Version/body_thin.stl'],
  ['choc/body_thin_panel.stl', 'Choc_Version/body_thin_panel.stl'],
  ['choc/face_panel.stl', 'Choc_Version/face_panel.stl'],
  ['choc/reset_button.stl', 'Choc_Version/reset_button.stl'],
  ['touchpad/right_touchpad_body.stl', 'Choc_Version/right_touchpad_body.stl'],
  ['touchpad/right_touchpad_cover.stl', 'Choc_Version/right_touchpad_cover.stl'],
  ['mx/board_case.stl', 'MX_Version/board_case.stl'],
  ['mx/board_cover.stl', 'MX_Version/board_cover.stl'],
  ['mx/body_thick.stl', 'MX_Version/body_thick.stl'],
  ['mx/body_thin.stl', 'MX_Version/body_thin.stl'],
  ['dongle/dongle_breadboard_bottom.stl', 'dongle/dongle_breadboard_bottom.stl'],
  ['dongle/dongle_breadboard_upper.stl', 'dongle/dongle_breadboard_upper.stl'],
  ['dongle/dongle_breadboard_button.stl', 'dongle/dongle_breadboard_button.stl'],
  ['legacy/corne-chocoflan-case.stl', 'corne-chocoflan-case.stl'],
  ['legacy/corne_chocoflan_basic.stl', 'corne_chocoflan_basic.stl'],
  ['legacy/body_with_stands.stl', 'body_with_stands.stl'],
  ['legacy/tbenen-plate.stl', 'tbenen-plate.stl'],
].map(([target, source]) => Object.freeze({ target, source })));

import { style } from "@vanilla-extract/css";
import { themeColors } from "../theme.css";

export const main = style({
  flexGrow: '1',
  flexShrink: '1',
  flexBasis: 'auto',
  borderColor: themeColors.foreground,
  borderWidth: '1px',
  borderStyle: 'solid',
})

import { style } from "@vanilla-extract/css";
import { themeColors } from "../theme.css";

export const container = style({
  fontSize: "x-large",
  paddingBottom: "0.5em",
  paddingTop: "0.5em",
});

export const text = style({
  color: themeColors.foreground,
  cursor: "pointer",
  textDecoration: "none",
  ':hover': {
    color: themeColors.focusedText,
    textDecoration: "underline",
  }
});

import { style } from "@vanilla-extract/css";
import { themeColors } from "./theme.css";

export const app = style({
  margin: "auto",
  width: "90%",
  height: "90vh",
});

export const main = style({
  height: "95%",
  display: "flex",
  flexWrap: "nowrap",
  flexDirection: "row",
  justifyContent: "center",
  alignItems: "stretch",
  columnGap: "10px",
})

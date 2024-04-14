import { globalStyle, createVar } from "@vanilla-extract/css";

export const themeColors = {
  focusedText: createVar(),
  unfocusedText: "#888888",
  background: createVar(),
  foreground: createVar(),
};

globalStyle(":root", {
  fontFamily: "Inter, system-ui, Avenir, Helvetica, Arial, sans-serif",
  color: themeColors.foreground,
  backgroundColor: themeColors.background,
  fontSynthesis: "none",
  textRendering: "optimizeLegibility",
  "@media": {
    // colors for light theme
    "(prefers-color-scheme: light)": {
      vars: {
        [themeColors.focusedText]: "#000000",
        [themeColors.background]: "#ffffff",
        [themeColors.foreground]: "#000000",
      },
    },
    // colors for dark theme
    "(prefers-color-scheme: dark)": {
      vars: {
        [themeColors.focusedText]: "#ffff00",
        [themeColors.background]: "#242424",
        [themeColors.foreground]: "#ffffff",
      },
    },
  },
});


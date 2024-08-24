import { config } from "./config";
import { InputText } from "./component/inputText";
import { InputRange } from "./component/inputRange";
import { syncCanvasSize, updateCanvas } from "./component/canvas";
import { initMenuItems } from "./component/menu";

// instance of UI components
const screenSize = {
  width: new InputText("screen-width"),
  height: new InputText("screen-height"),
};
const distance = {
  screen: new InputText("focal-screen-distance"),
  camera: new InputText("focal-camera-distance"),
};
const boxSize = {
  x: new InputText("box-size-x"),
  y: new InputText("box-size-y"),
  z: new InputText("box-size-z"),
};
const angle = {
  elevation: new InputRange("elevation"),
  azimuth: new InputRange("azimuth"),
  roll: new InputRange("roll"),
};

// event handler for each instance
// it is
//   1. registered, which is for when the input is updated
//   2. invoked when the window is loaded
const handleScreenSizeInput = {
  width: (inputValue: string): void => {
    config.screenWidth = Number(inputValue);
    updateCanvas(config);
  },
  height: (inputValue: string): void => {
    config.screenHeight = Number(inputValue);
    updateCanvas(config);
  },
};
const handleDistanceInput = {
  screen: (inputValue: string): void => {
    config.focalScreenDistance = Number(inputValue);
    updateCanvas(config);
  },
  camera: (inputValue: string): void => {
    config.focalCameraDistance = Number(inputValue);
    updateCanvas(config);
  },
};
const handleBoxSizeInput = {
  x: (inputValue: string): void => {
    config.boxSizeX = Number(inputValue);
    updateCanvas(config);
  },
  y: (inputValue: string): void => {
    config.boxSizeY = Number(inputValue);
    updateCanvas(config);
  },
  z: (inputValue: string): void => {
    config.boxSizeZ = Number(inputValue);
    updateCanvas(config);
  },
};
const handleAngleInput = {
  elevation: (inputValue: string): void => {
    config.elevation = Number(inputValue);
    updateCanvas(config);
  },
  azimuth: (inputValue: string): void => {
    config.azimuth = Number(inputValue);
    updateCanvas(config);
  },
  roll: (inputValue: string): void => {
    config.roll = Number(inputValue);
    updateCanvas(config);
  },
};

screenSize.width.onValidInput(handleScreenSizeInput.width);
screenSize.height.onValidInput(handleScreenSizeInput.height);
distance.screen.onValidInput(handleDistanceInput.screen);
distance.camera.onValidInput(handleDistanceInput.camera);
boxSize.x.onValidInput(handleBoxSizeInput.x);
boxSize.y.onValidInput(handleBoxSizeInput.y);
boxSize.z.onValidInput(handleBoxSizeInput.z);
angle.elevation.onInput(handleAngleInput.elevation);
angle.azimuth.onInput(handleAngleInput.azimuth);
angle.roll.onInput(handleAngleInput.roll);

window.addEventListener("load", () => {
  initMenuItems();
  // canvas size should be in sync with the element size
  syncCanvasSize();
  // assign default DOM values to the config object
  screenSize.width.onWindowLoad(handleScreenSizeInput.width);
  screenSize.height.onWindowLoad(handleScreenSizeInput.height);
  distance.screen.onWindowLoad(handleDistanceInput.screen);
  distance.camera.onWindowLoad(handleDistanceInput.camera);
  boxSize.x.onWindowLoad(handleBoxSizeInput.x);
  boxSize.y.onWindowLoad(handleBoxSizeInput.y);
  boxSize.z.onWindowLoad(handleBoxSizeInput.z);
  angle.elevation.onWindowLoad(handleAngleInput.elevation);
  angle.azimuth.onWindowLoad(handleAngleInput.azimuth);
  angle.roll.onWindowLoad(handleAngleInput.roll);
  // initial draw
  updateCanvas(config);
});

window.addEventListener("resize", () => {
  // when window is resized, the canvas element (flex item) is resized as well,
  //   which alters the aspect ratio of the canvas
  // to avoid the image distortion, the internal canvas size
  //   (canvas.width, canvas.height) should be updated accordingly,
  //   and of course everything should be re-drawn
  syncCanvasSize();
  updateCanvas(config);
});

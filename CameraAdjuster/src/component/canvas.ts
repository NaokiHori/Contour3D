import { getElementByIdUnwrap } from "./getElement";
import { Vector3D } from "../vector";
import { Config } from "../config";
import { BoundingBox, getBoundingBox } from "./canvas/boundingBox";
import { drawCaption } from "./canvas/drawCaption";
import { drawEdges } from "./canvas/drawEdges";
import { drawLineSegments } from "./canvas/drawLineSegments";
import { drawAxes } from "./canvas/drawAxes";
import { showModal } from "./modal";

export const canvas = getElementByIdUnwrap("main-canvas") as HTMLCanvasElement;

canvas.addEventListener("click", () => {
  showModal();
});

// sync internal size of canvas with the displayed size
export function syncCanvasSize() {
  // get displayed canvas sizes
  const rect: DOMRect = canvas.getBoundingClientRect();
  const displayWidth: number = rect.width;
  const displayHeight: number = rect.height;
  // update internal canvas size
  // NOTE: assuming no borders or "box-sizing: border-box"
  canvas.width = displayWidth;
  canvas.height = displayHeight;
}

export function updateCanvas(config: Config) {
  const boundingBox: BoundingBox = getBoundingBox(
    canvas,
    config.screenWidth / config.screenHeight,
  );
  const screenWidth: number = boundingBox.imax - boundingBox.imin + 1;
  const screenHeight: number = boundingBox.jmax - boundingBox.jmin + 1;
  const ctx: CanvasRenderingContext2D | null = canvas.getContext("2d");
  if (ctx === null) {
    throw new Error("failed to get context");
  }
  const cameraPosition: Vector3D = config.cameraPosition;
  const {
    horizontal: screenHorizontal,
    vertical: screenVertical,
    normal: screenNormal,
  }: {
    horizontal: Vector3D;
    vertical: Vector3D;
    normal: Vector3D;
  } = config.screenVectors;
  const screenCenter: Vector3D = config.screenCenter;
  // draw
  ctx.clearRect(0, 0, canvas.width, canvas.height);
  if (screenWidth <= 0 || screenHeight <= 0) {
    return;
  }
  const imageData: ImageData = ctx.createImageData(screenWidth, screenHeight);
  drawLineSegments(
    cameraPosition,
    screenHorizontal,
    screenVertical,
    screenNormal,
    screenCenter,
    {
      x: config.boxSizeX,
      y: config.boxSizeY,
      z: config.boxSizeZ,
    } satisfies Vector3D,
    imageData,
  );
  drawAxes(
    cameraPosition,
    screenHorizontal,
    screenVertical,
    screenNormal,
    screenCenter,
    imageData,
  );
  drawEdges(imageData);
  ctx.putImageData(imageData, boundingBox.imin, boundingBox.jmin);
  drawCaption(ctx);
}

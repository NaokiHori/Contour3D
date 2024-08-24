import { Vector3D } from "../../vector";
import { LineSegment, drawLineSegment } from "./lineSegment";
import { Color } from "./color";

export function drawAxes(
  cameraPosition: Vector3D,
  screenHorizontal: Vector3D,
  screenVertical: Vector3D,
  screenNormal: Vector3D,
  screenCenter: Vector3D,
  imageData: ImageData,
) {
  const width: number = imageData.width;
  const height: number = imageData.height;
  const axes: { lineSegment: LineSegment; color: Color }[] = [
    {
      lineSegment: {
        s: { x: 0, y: 0, z: 0 },
        e: { x: 1, y: 0, z: 0 },
      },
      color: { r: 255, g: 0, b: 255, a: 255 },
    },
    {
      lineSegment: {
        s: { x: 0, y: 0, z: 0 },
        e: { x: 0, y: 1, z: 0 },
      },
      color: { r: 255, g: 215, b: 0, a: 255 },
    },
    {
      lineSegment: {
        s: { x: 0, y: 0, z: 0 },
        e: { x: 0, y: 0, z: 1 },
      },
      color: { r: 0, g: 255, b: 255, a: 255 },
    },
  ];
  for (const axis of axes) {
    drawLineSegment(
      cameraPosition,
      screenHorizontal,
      screenVertical,
      screenNormal,
      screenCenter,
      width,
      height,
      imageData,
      axis.lineSegment,
      axis.color,
    );
  }
}

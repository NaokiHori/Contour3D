import { Vector3D } from "../../vector";
import { LineSegment, drawLineSegment } from "./lineSegment";
import { Color } from "./color";

export function drawLineSegments(
  cameraPosition: Vector3D,
  screenHorizontal: Vector3D,
  screenVertical: Vector3D,
  screenNormal: Vector3D,
  screenCenter: Vector3D,
  boxSizes: Vector3D,
  imageData: ImageData,
) {
  const width: number = imageData.width;
  const height: number = imageData.height;
  const color: Color = { r: 255, g: 255, b: 255, a: 255 };
  const lineSegments: LineSegment[] = [
    // in x
    {
      s: { x: -0.5 * boxSizes.x, y: -0.5 * boxSizes.y, z: -0.5 * boxSizes.z },
      e: { x: +0.5 * boxSizes.x, y: -0.5 * boxSizes.y, z: -0.5 * boxSizes.z },
    },
    {
      s: { x: -0.5 * boxSizes.x, y: +0.5 * boxSizes.y, z: -0.5 * boxSizes.z },
      e: { x: +0.5 * boxSizes.x, y: +0.5 * boxSizes.y, z: -0.5 * boxSizes.z },
    },
    {
      s: { x: -0.5 * boxSizes.x, y: -0.5 * boxSizes.y, z: +0.5 * boxSizes.z },
      e: { x: +0.5 * boxSizes.x, y: -0.5 * boxSizes.y, z: +0.5 * boxSizes.z },
    },
    {
      s: { x: -0.5 * boxSizes.x, y: +0.5 * boxSizes.y, z: +0.5 * boxSizes.z },
      e: { x: +0.5 * boxSizes.x, y: +0.5 * boxSizes.y, z: +0.5 * boxSizes.z },
    },
    // in y
    {
      s: { x: -0.5 * boxSizes.x, y: -0.5 * boxSizes.y, z: -0.5 * boxSizes.z },
      e: { x: -0.5 * boxSizes.x, y: +0.5 * boxSizes.y, z: -0.5 * boxSizes.z },
    },
    {
      s: { x: +0.5 * boxSizes.x, y: -0.5 * boxSizes.y, z: -0.5 * boxSizes.z },
      e: { x: +0.5 * boxSizes.x, y: +0.5 * boxSizes.y, z: -0.5 * boxSizes.z },
    },
    {
      s: { x: -0.5 * boxSizes.x, y: -0.5 * boxSizes.y, z: +0.5 * boxSizes.z },
      e: { x: -0.5 * boxSizes.x, y: +0.5 * boxSizes.y, z: +0.5 * boxSizes.z },
    },
    {
      s: { x: +0.5 * boxSizes.x, y: -0.5 * boxSizes.y, z: +0.5 * boxSizes.z },
      e: { x: +0.5 * boxSizes.x, y: +0.5 * boxSizes.y, z: +0.5 * boxSizes.z },
    },
    // in z
    {
      s: { x: -0.5 * boxSizes.x, y: -0.5 * boxSizes.y, z: -0.5 * boxSizes.z },
      e: { x: -0.5 * boxSizes.x, y: -0.5 * boxSizes.y, z: +0.5 * boxSizes.z },
    },
    {
      s: { x: +0.5 * boxSizes.x, y: -0.5 * boxSizes.y, z: -0.5 * boxSizes.z },
      e: { x: +0.5 * boxSizes.x, y: -0.5 * boxSizes.y, z: +0.5 * boxSizes.z },
    },
    {
      s: { x: -0.5 * boxSizes.x, y: +0.5 * boxSizes.y, z: -0.5 * boxSizes.z },
      e: { x: -0.5 * boxSizes.x, y: +0.5 * boxSizes.y, z: +0.5 * boxSizes.z },
    },
    {
      s: { x: +0.5 * boxSizes.x, y: +0.5 * boxSizes.y, z: -0.5 * boxSizes.z },
      e: { x: +0.5 * boxSizes.x, y: +0.5 * boxSizes.y, z: +0.5 * boxSizes.z },
    },
  ];
  for (const lineSegment of lineSegments) {
    drawLineSegment(
      cameraPosition,
      screenHorizontal,
      screenVertical,
      screenNormal,
      screenCenter,
      width,
      height,
      imageData,
      lineSegment,
      color,
    );
  }
}

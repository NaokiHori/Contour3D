export interface BoundingBox {
  imin: number;
  imax: number;
  jmin: number;
  jmax: number;
}

function clipOutOfBounds(
  canvasWidth: number,
  canvasHeight: number,
  boundingBox: BoundingBox,
): BoundingBox {
  return {
    imin: Math.max(boundingBox.imin, 0),
    imax: Math.min(boundingBox.imax, canvasWidth - 1),
    jmin: Math.max(boundingBox.jmin, 0),
    jmax: Math.min(boundingBox.jmax, canvasHeight - 1),
  };
}

export function getBoundingBox(
  canvas: HTMLCanvasElement,
  screenAspectRatio: number,
): BoundingBox {
  const canvasWidth: number = canvas.width;
  const canvasHeight: number = canvas.height;
  const canvasAspectRatio: number = canvasWidth / canvasHeight;
  if (screenAspectRatio < canvasAspectRatio) {
    const gap = 0.5 * (canvasWidth - canvasHeight * screenAspectRatio);
    return clipOutOfBounds(canvasWidth, canvasHeight, {
      imin: Math.round(gap),
      imax: Math.round(gap + canvasHeight * screenAspectRatio),
      jmin: Math.round(0),
      jmax: Math.round(canvasHeight - 1),
    } satisfies BoundingBox);
  } else {
    return clipOutOfBounds(canvasWidth, canvasHeight, {
      imin: Math.round(0),
      imax: Math.round(canvasWidth - 1),
      jmin: Math.round(0),
      jmax: Math.round(canvasWidth / screenAspectRatio),
    } satisfies BoundingBox);
  }
}

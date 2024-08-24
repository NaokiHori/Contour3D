export function drawEdges(imageData: ImageData) {
  const data: Uint8ClampedArray = imageData.data;
  const width: number = imageData.width;
  const height: number = imageData.height;
  for (let j = 0; j < height; j++) {
    for (const i of [0, width - 1]) {
      data[(j * width + i) * 4 + 0] = 255;
      data[(j * width + i) * 4 + 1] = 255;
      data[(j * width + i) * 4 + 2] = 255;
      data[(j * width + i) * 4 + 3] = 255;
    }
  }
  for (const j of [0, height - 1]) {
    for (let i = 0; i < width; i++) {
      data[(j * width + i) * 4 + 0] = 255;
      data[(j * width + i) * 4 + 1] = 255;
      data[(j * width + i) * 4 + 2] = 255;
      data[(j * width + i) * 4 + 3] = 255;
    }
  }
}

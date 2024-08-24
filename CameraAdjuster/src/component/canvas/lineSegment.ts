import { Vector2D, Vector3D, innerProduct } from "../../vector";
import { Color } from "./color";

export interface LineSegment {
  s: Vector3D;
  e: Vector3D;
}

function project(
  cameraPosition: Vector3D,
  screenHorizontal: Vector3D,
  screenVertical: Vector3D,
  screenNormal: Vector3D,
  screenCenter: Vector3D,
  width: number,
  height: number,
  p: Vector3D,
): Vector3D | null {
  const ray: Vector3D = {
    x: p.x - cameraPosition.x,
    y: p.y - cameraPosition.y,
    z: p.z - cameraPosition.z,
  };
  const nc: number = innerProduct(screenNormal, cameraPosition);
  const nr: number = innerProduct(screenNormal, ray);
  const d: number = innerProduct(screenNormal, screenCenter);
  const small = 1.0e-8;
  if (-small < nr && nr < small) {
    return null;
  }
  const t: number = (d - nc) / nr;
  if (t <= 0 || 1 <= t) {
    return null;
  }
  const delta: Vector3D = {
    x: cameraPosition.x + t * ray.x - screenCenter.x,
    y: cameraPosition.y + t * ray.y - screenCenter.y,
    z: cameraPosition.z + t * ray.z - screenCenter.z,
  };
  return {
    x:
      (0.5 +
        innerProduct(delta, screenHorizontal) /
          innerProduct(screenHorizontal, screenHorizontal)) *
      width,
    y:
      (0.5 +
        innerProduct(delta, screenVertical) /
          innerProduct(screenVertical, screenVertical)) *
      height,
    z: nr,
  };
}

function findDistance(a: Vector2D, b: Vector2D, p: Vector2D): number {
  const ba: Vector2D = {
    x: b.x - a.x,
    y: b.y - a.y,
  };
  const pa: Vector2D = {
    x: p.x - a.x,
    y: p.y - a.y,
  };
  const pb: Vector2D = {
    x: p.x - b.x,
    y: p.y - b.y,
  };
  const t: number = innerProduct(pa, ba) / innerProduct(ba, ba);
  if (t < 0) {
    return Math.sqrt(innerProduct(pa, pa));
  } else if (1 < t) {
    return Math.sqrt(innerProduct(pb, pb));
  } else {
    const q: Vector2D = {
      x: a.x + t * ba.x,
      y: a.y + t * ba.y,
    };
    const pq: Vector2D = {
      x: q.x - p.x,
      y: q.y - p.y,
    };
    return Math.sqrt(innerProduct(pq, pq));
  }
}

function blurLine(w: number, d: number): number {
  const beta = 1.5;
  return 0.5 * (1 - Math.tanh(beta * (d - w)));
}

export function drawLineSegment(
  cameraPosition: Vector3D,
  screenHorizontal: Vector3D,
  screenVertical: Vector3D,
  screenNormal: Vector3D,
  screenCenter: Vector3D,
  width: number,
  height: number,
  imageData: ImageData,
  lineSegment: LineSegment,
  lineColor: Color,
) {
  const data: Uint8ClampedArray = imageData.data;
  const nitems = 16;
  const lineWidth = 3;
  const deltas: Vector3D = {
    x: (lineSegment.e.x - lineSegment.s.x) / (nitems - 1),
    y: (lineSegment.e.y - lineSegment.s.y) / (nitems - 1),
    z: (lineSegment.e.z - lineSegment.s.z) / (nitems - 1),
  };
  for (let n = 0; n < nitems - 1; n++) {
    const s: Vector3D = {
      x: lineSegment.s.x + (n + 0) * deltas.x,
      y: lineSegment.s.y + (n + 0) * deltas.y,
      z: lineSegment.s.z + (n + 0) * deltas.z,
    };
    const e: Vector3D = {
      x: lineSegment.s.x + (n + 1) * deltas.x,
      y: lineSegment.s.y + (n + 1) * deltas.y,
      z: lineSegment.s.z + (n + 1) * deltas.z,
    };
    const projected_s: Vector3D | null = project(
      cameraPosition,
      screenHorizontal,
      screenVertical,
      screenNormal,
      screenCenter,
      width,
      height,
      s,
    );
    const projected_e: Vector3D | null = project(
      cameraPosition,
      screenHorizontal,
      screenVertical,
      screenNormal,
      screenCenter,
      width,
      height,
      e,
    );
    if (null === projected_s || null === projected_e) {
      continue;
    }
    const xmin: number = Math.min(projected_s.x, projected_e.x) - lineWidth;
    const xmax: number = Math.max(projected_s.x, projected_e.x) + lineWidth;
    const ymin: number = Math.min(projected_s.y, projected_e.y) - lineWidth;
    const ymax: number = Math.max(projected_s.y, projected_e.y) + lineWidth;
    const imin: number = Math.floor(Math.min(width - 1, Math.max(0, xmin)));
    const imax: number = Math.ceil(Math.min(width - 1, Math.max(0, xmax)));
    const jmin: number = Math.floor(Math.min(height - 1, Math.max(0, ymin)));
    const jmax: number = Math.ceil(Math.min(height - 1, Math.max(0, ymax)));
    for (let j = jmin; j <= jmax; j++) {
      for (let i = imin; i <= imax; i++) {
        const d: number = findDistance(
          {
            x: projected_s.x,
            y: projected_s.y,
          },
          {
            x: projected_e.x,
            y: projected_e.y,
          },
          {
            x: i,
            y: j,
          },
        );
        const rate: number = blurLine(lineWidth, d);
        const rgba: [number, number, number, number] = [
          rate * lineColor.r,
          rate * lineColor.g,
          rate * lineColor.b,
          rate * lineColor.a,
        ];
        for (let k = 0; k < 4; k++) {
          // NOTE: upside-down on screen
          const index: number = 4 * ((height - 1 - j) * width + i) + k;
          data[index] = Math.max(data[index], rgba[k]);
        }
      }
    }
  }
}

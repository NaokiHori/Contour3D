export interface Vector2D {
  x: number;
  y: number;
}

export interface Vector3D {
  x: number;
  y: number;
  z: number;
}

type Vector = Vector2D | Vector3D;

export function innerProduct(v0: Vector, v1: Vector): number {
  if ("z" in v0 && "z" in v1) {
    return v0.x * v1.x + v0.y * v1.y + v0.z * v1.z;
  } else {
    return v0.x * v1.x + v0.y * v1.y;
  }
}

export function crossProduct(v0: Vector3D, v1: Vector3D): Vector3D {
  return {
    x: v0.y * v1.z - v0.z * v1.y,
    y: v0.z * v1.x - v0.x * v1.z,
    z: v0.x * v1.y - v0.y * v1.x,
  };
}

export function normalize(v: Vector3D): Vector3D {
  const norminv: number = 1 / Math.sqrt(innerProduct(v, v));
  return {
    x: v.x * norminv,
    y: v.y * norminv,
    z: v.z * norminv,
  };
}

export function rodrigues(n: Vector3D, t: number, b: Vector3D): Vector3D {
  // Rodrigues' rotation formula:
  //   rotate a vector "b" by the angle "t" (in radian) around a vector "n"
  // since the length of the rotation vector may not be unity,
  //   I normalise it first
  n = normalize(n);
  // prepare 3x3 rotation matrix
  const cost: number = Math.cos(t);
  const sint: number = Math.sin(t);
  const a00: number = n.x * n.x * (1 - cost) + cost;
  const a01: number = n.x * n.y * (1 - cost) - n.z * sint;
  const a02: number = n.x * n.z * (1 - cost) + n.y * sint;
  const a10: number = n.y * n.x * (1 - cost) + n.z * sint;
  const a11: number = n.y * n.y * (1 - cost) + cost;
  const a12: number = n.y * n.z * (1 - cost) - n.x * sint;
  const a20: number = n.z * n.x * (1 - cost) - n.y * sint;
  const a21: number = n.z * n.y * (1 - cost) + n.x * sint;
  const a22: number = n.z * n.z * (1 - cost) + cost;
  return {
    x: a00 * b.x + a01 * b.y + a02 * b.z,
    y: a10 * b.x + a11 * b.y + a12 * b.z,
    z: a20 * b.x + a21 * b.y + a22 * b.z,
  };
}

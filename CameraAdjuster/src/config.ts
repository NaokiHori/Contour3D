import { Vector3D, crossProduct, normalize, rodrigues } from "./vector";

function degreeToRadian(degree: number): number {
  return (Math.PI * degree) / 180;
}

const ex: Vector3D = {
  x: 1,
  y: 0,
  z: 0,
};

const ez: Vector3D = {
  x: 0,
  y: 0,
  z: 1,
};

export class Config {
  // angles
  // NOTE: in degree, NOT in radian
  private _angle: {
    elevation: number;
    azimuth: number;
    roll: number;
  };
  // screen size
  private _screen: {
    width: number;
    height: number;
  };
  // distance
  private _focalScreenDistance: number;
  private _focalCameraDistance: number;
  // box sizes
  private _boxSizeX: number;
  private _boxSizeY: number;
  private _boxSizeZ: number;

  public constructor() {
    // NOTE: give tentative default values,
    //         which will soon be overriden
    //         by loading HTMLInputElements
    this._angle = {
      elevation: 0,
      azimuth: 0,
      roll: 0,
    };
    this._screen = {
      width: 1,
      height: 1,
    };
    this._focalScreenDistance = 1;
    this._focalCameraDistance = 2;
    this._boxSizeX = 1;
    this._boxSizeY = 1;
    this._boxSizeZ = 1;
  }

  public get elevation(): number {
    return this._angle.elevation;
  }

  public set elevation(elevation: number) {
    this._angle.elevation = elevation;
  }

  public get azimuth(): number {
    return this._angle.azimuth;
  }

  public set azimuth(azimuth: number) {
    this._angle.azimuth = azimuth;
  }

  public get roll(): number {
    return this._angle.roll;
  }

  public set roll(roll: number) {
    this._angle.roll = roll;
  }

  public get boxSizeX(): number {
    return this._boxSizeX;
  }

  public set boxSizeX(boxSizeX: number) {
    this._boxSizeX = boxSizeX;
  }

  public get boxSizeY(): number {
    return this._boxSizeY;
  }

  public set boxSizeY(boxSizeY: number) {
    this._boxSizeY = boxSizeY;
  }

  public get boxSizeZ(): number {
    return this._boxSizeZ;
  }

  public set boxSizeZ(boxSizeZ: number) {
    this._boxSizeZ = boxSizeZ;
  }

  public get screenWidth(): number {
    return this._screen.width;
  }

  public set screenWidth(screenWidth: number) {
    this._screen.width = screenWidth;
  }

  public get screenHeight(): number {
    return this._screen.height;
  }

  public set screenHeight(screenHeight: number) {
    this._screen.height = screenHeight;
  }

  public get focalScreenDistance(): number {
    return this._focalScreenDistance;
  }

  public set focalScreenDistance(focalScreenDistance: number) {
    this._focalScreenDistance = focalScreenDistance;
  }

  public get focalCameraDistance(): number {
    return this._focalCameraDistance;
  }

  public set focalCameraDistance(focalCameraDistance: number) {
    this._focalCameraDistance = focalCameraDistance;
  }

  public get cameraPosition(): Vector3D {
    const elevation: number = degreeToRadian(this._angle.elevation);
    const azimuth: number = degreeToRadian(this._angle.azimuth);
    const focalCameraDistance: number = this._focalCameraDistance;
    let cameraPosition: Vector3D = { x: 0, y: 0, z: focalCameraDistance };
    cameraPosition = rodrigues(ex, elevation, cameraPosition);
    cameraPosition = rodrigues(ez, azimuth, cameraPosition);
    return cameraPosition;
  }

  public get screenVectors(): {
    horizontal: Vector3D;
    vertical: Vector3D;
    normal: Vector3D;
  } {
    const elevation: number = degreeToRadian(this._angle.elevation);
    const azimuth: number = degreeToRadian(this._angle.azimuth);
    const roll: number = degreeToRadian(this._angle.roll);
    const screenWidth: number = this._screen.width;
    const screenHeight: number = this._screen.height;
    // initial vectors on xy plane
    let screenHorizontal: Vector3D = { x: screenWidth, y: 0, z: 0 };
    let screenVertical: Vector3D = { x: 0, y: screenHeight, z: 0 };
    // elevation: rotate around ex
    screenHorizontal = rodrigues(ex, elevation, screenHorizontal);
    screenVertical = rodrigues(ex, elevation, screenVertical);
    // azimuth: rotate around ez
    screenHorizontal = rodrigues(ez, azimuth, screenHorizontal);
    screenVertical = rodrigues(ez, azimuth, screenVertical);
    // roll: rorate around screen normal
    const screenNormal: Vector3D = normalize(
      crossProduct(screenHorizontal, screenVertical),
    );
    screenHorizontal = rodrigues(screenNormal, roll, screenHorizontal);
    screenVertical = rodrigues(screenNormal, roll, screenVertical);
    return {
      horizontal: screenHorizontal,
      vertical: screenVertical,
      normal: screenNormal,
    };
  }

  public get screenCenter(): Vector3D {
    const focalScreenDistance: number = this._focalScreenDistance;
    const focalCameraDistance: number = this._focalCameraDistance;
    const rate: number = focalScreenDistance / focalCameraDistance;
    const cameraPosition: Vector3D = this.cameraPosition;
    return {
      x: rate * cameraPosition.x,
      y: rate * cameraPosition.y,
      z: rate * cameraPosition.z,
    };
  }
}

// an object to store camera and screen configuration
export const config = new Config();

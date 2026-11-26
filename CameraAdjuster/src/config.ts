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
  private angle: {
    elevation: number;
    azimuth: number;
    roll: number;
  };
  // screen size
  private screen: {
    width: number;
    height: number;
  };
  // distance
  private focalScreenDistance: number;
  private focalCameraDistance: number;
  // box sizes
  private boxSizeX: number;
  private boxSizeY: number;
  private boxSizeZ: number;

  public constructor() {
    // NOTE: give tentative default values,
    //         which will soon be overriden
    //         by loading HTMLInputElements
    this.angle = {
      elevation: 0,
      azimuth: 0,
      roll: 0,
    };
    this.screen = {
      width: 1,
      height: 1,
    };
    this.focalScreenDistance = 1;
    this.focalCameraDistance = 2;
    this.boxSizeX = 1;
    this.boxSizeY = 1;
    this.boxSizeZ = 1;
  }

  public getElevation(): number {
    return this.angle.elevation;
  }

  public setElevation(elevation: number) {
    this.angle.elevation = elevation;
  }

  public getAzimuth(): number {
    return this.angle.azimuth;
  }

  public setAzimuth(azimuth: number) {
    this.angle.azimuth = azimuth;
  }

  public getRoll(): number {
    return this.angle.roll;
  }

  public setRoll(roll: number) {
    this.angle.roll = roll;
  }

  public getBoxSizeX(): number {
    return this.boxSizeX;
  }

  public setBoxSizeX(boxSizeX: number) {
    this.boxSizeX = boxSizeX;
  }

  public getBoxSizeY(): number {
    return this.boxSizeY;
  }

  public setBoxSizeY(boxSizeY: number) {
    this.boxSizeY = boxSizeY;
  }

  public getBoxSizeZ(): number {
    return this.boxSizeZ;
  }

  public setBoxSizeZ(boxSizeZ: number) {
    this.boxSizeZ = boxSizeZ;
  }

  public getScreenWidth(): number {
    return this.screen.width;
  }

  public setScreenWidth(screenWidth: number) {
    this.screen.width = screenWidth;
  }

  public getScreenHeight(): number {
    return this.screen.height;
  }

  public setScreenHeight(screenHeight: number) {
    this.screen.height = screenHeight;
  }

  public getFocalScreenDistance(): number {
    return this.focalScreenDistance;
  }

  public setFocalScreenDistance(focalScreenDistance: number) {
    this.focalScreenDistance = focalScreenDistance;
  }

  public getFocalCameraDistance(): number {
    return this.focalCameraDistance;
  }

  public setFocalCameraDistance(focalCameraDistance: number) {
    this.focalCameraDistance = focalCameraDistance;
  }

  public getCameraPosition(): Vector3D {
    const elevation: number = degreeToRadian(this.angle.elevation);
    const azimuth: number = degreeToRadian(this.angle.azimuth);
    const focalCameraDistance: number = this.focalCameraDistance;
    let cameraPosition: Vector3D = { x: 0, y: 0, z: focalCameraDistance };
    cameraPosition = rodrigues(ex, elevation, cameraPosition);
    cameraPosition = rodrigues(ez, azimuth, cameraPosition);
    return cameraPosition;
  }

  public getScreenVectors(): {
    horizontal: Vector3D;
    vertical: Vector3D;
    normal: Vector3D;
  } {
    const elevation: number = degreeToRadian(this.angle.elevation);
    const azimuth: number = degreeToRadian(this.angle.azimuth);
    const roll: number = degreeToRadian(this.angle.roll);
    const screenWidth: number = this.screen.width;
    const screenHeight: number = this.screen.height;
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
    const screenNormal: Vector3D = normalize(crossProduct(screenHorizontal, screenVertical));
    screenHorizontal = rodrigues(screenNormal, roll, screenHorizontal);
    screenVertical = rodrigues(screenNormal, roll, screenVertical);
    return {
      horizontal: screenHorizontal,
      vertical: screenVertical,
      normal: screenNormal,
    };
  }

  public getScreenCenter(): Vector3D {
    const focalScreenDistance: number = this.focalScreenDistance;
    const focalCameraDistance: number = this.focalCameraDistance;
    const rate: number = focalScreenDistance / focalCameraDistance;
    const cameraPosition: Vector3D = this.getCameraPosition();
    return {
      x: rate * cameraPosition.x,
      y: rate * cameraPosition.y,
      z: rate * cameraPosition.z,
    };
  }
}

// an object to store camera and screen configuration
export const config = new Config();

import { Vector3D } from "../vector";
import { config } from "../config";
import { getElementByIdOrThrow } from "./getElement";
import { classNameValidInput, classNameInvalidInput, isPositiveNumber } from "./validity";

function formatFloat(value: number): string {
  // "%+.15e"
  const sign: string = value < 0 ? "" : "+";
  return sign + value.toExponential(15);
}

function makeCodeSpace() {
  const pixelsPerUnitLength: string = pixelsPerUnitLengthInput.value;
  const isValid: boolean = isPositiveNumber(pixelsPerUnitLength);
  if (!isValid) {
    pixelsPerUnitLengthInput.className = classNameInvalidInput;
    return;
  }
  pixelsPerUnitLengthInput.className = classNameValidInput;
  const screenWidth: number = Math.round(config.getScreenWidth() * Number(pixelsPerUnitLength));
  const screenHeight: number = Math.round(config.getScreenHeight() * Number(pixelsPerUnitLength));
  //
  const cameraPosition: Vector3D = config.getCameraPosition();
  const {
    horizontal: screenHorizontal,
    vertical: screenVertical,
  }: {
    horizontal: Vector3D;
    vertical: Vector3D;
  } = config.getScreenVectors();
  const screenCenter: Vector3D = config.getScreenCenter();
  //
  const vectorType = "contour3d_vector_t";
  let code = "";
  code += `<pre>`;
  code += `const ${vectorType} camera_position = {\n`;
  code += `  .x = ${formatFloat(cameraPosition.x)},\n`;
  code += `  .y = ${formatFloat(cameraPosition.y)},\n`;
  code += `  .z = ${formatFloat(cameraPosition.z)},\n`;
  code += `};\n`;
  code += `const ${vectorType} camera_look_at = {\n`;
  code += `  .x = ${formatFloat(0)},\n`;
  code += `  .y = ${formatFloat(0)},\n`;
  code += `  .z = ${formatFloat(0)},\n`;
  code += `};\n`;
  code += `const size_t screen_sizes[2] = {\n`;
  code += `  ${screenWidth.toString()},\n`;
  code += `  ${screenHeight.toString()},\n`;
  code += `};\n`;
  code += `const ${vectorType} screen_center = {\n`;
  code += `  .x = ${formatFloat(screenCenter.x)},\n`;
  code += `  .y = ${formatFloat(screenCenter.y)},\n`;
  code += `  .z = ${formatFloat(screenCenter.z)},\n`;
  code += `};\n`;
  code += `const ${vectorType} screen_local[2] = {\n`;
  code += `  {\n`;
  code += `    .x = ${formatFloat(screenHorizontal.x)},\n`;
  code += `    .y = ${formatFloat(screenHorizontal.y)},\n`;
  code += `    .z = ${formatFloat(screenHorizontal.z)},\n`;
  code += `  },\n`;
  code += `  {\n`;
  code += `    .x = ${formatFloat(screenVertical.x)},\n`;
  code += `    .y = ${formatFloat(screenVertical.y)},\n`;
  code += `    .z = ${formatFloat(screenVertical.z)},\n`;
  code += `  },\n`;
  code += `};`;
  code += `</pre>`;
  codeSpace.innerHTML = code;
}

const modal = getElementByIdOrThrow(HTMLDialogElement, "code-modal");
const modalCloseButton = getElementByIdOrThrow(HTMLButtonElement, "code-modal-close-button");
const pixelsPerUnitLengthInput = getElementByIdOrThrow(HTMLInputElement, "pixels-per-unit-length");
const codeSpace = getElementByIdOrThrow(HTMLElement, "code-space");

pixelsPerUnitLengthInput.addEventListener("input", () => {
  makeCodeSpace();
});

export function showModal() {
  makeCodeSpace();
  modal.showModal();
}

modalCloseButton.addEventListener("click", () => {
  modal.close();
});

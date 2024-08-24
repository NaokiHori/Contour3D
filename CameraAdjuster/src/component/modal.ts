import { Vector3D } from "../vector";
import { config } from "../config";
import { getElementByIdUnwrap } from "./getElement";
import {
  classNameValidInput,
  classNameInvalidInput,
  isPositiveNumber,
} from "./validity";

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
  const screenWidth: number = Math.round(
    config.screenWidth * Number(pixelsPerUnitLength),
  );
  const screenHeight: number = Math.round(
    config.screenHeight * Number(pixelsPerUnitLength),
  );
  //
  const cameraPosition: Vector3D = config.cameraPosition;
  const {
    horizontal: screenHorizontal,
    vertical: screenVertical,
  }: {
    horizontal: Vector3D;
    vertical: Vector3D;
  } = config.screenVectors;
  const screenCenter: Vector3D = config.screenCenter;
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

const modal = getElementByIdUnwrap("code-modal") as HTMLDialogElement;
const modalCloseButton = getElementByIdUnwrap(
  "code-modal-close-button",
) as HTMLButtonElement;
const pixelsPerUnitLengthInput = getElementByIdUnwrap(
  "pixels-per-unit-length",
) as HTMLInputElement;
const codeSpace = getElementByIdUnwrap("code-space");

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

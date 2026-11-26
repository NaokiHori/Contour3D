import { getElementByIdOrThrow } from "./getElement";
import { classNameValidInput, classNameInvalidInput, isPositiveNumber } from "./validity";

export class InputText {
  private element: HTMLInputElement;

  public constructor(elementId: string) {
    this.element = getElementByIdOrThrow(HTMLInputElement, elementId);
  }

  public onWindowLoad(handler: (inputValue: string) => void) {
    this.validateAndUpdate(handler);
  }

  public onValidInput(handler: (inputValue: string) => void) {
    this.element.addEventListener("input", () => {
      this.validateAndUpdate(handler);
    });
  }

  private validateAndUpdate(handler: (inputValue: string) => void) {
    const value: string = this.element.value;
    const isValid: boolean = isPositiveNumber(value);
    if (isValid) {
      this.element.className = classNameValidInput;
      handler(value);
    } else {
      this.element.className = classNameInvalidInput;
    }
  }
}

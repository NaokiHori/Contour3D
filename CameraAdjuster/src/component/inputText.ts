import { getElementByIdUnwrap } from "./getElement";
import {
  classNameValidInput,
  classNameInvalidInput,
  isPositiveNumber,
} from "./validity";

export class InputText {
  private _element: HTMLInputElement;
  public constructor(elementId: string) {
    this._element = getElementByIdUnwrap(elementId) as HTMLInputElement;
  }
  public onWindowLoad(handler: (inputValue: string) => void) {
    this.validateAndUpdate(handler);
  }
  public onValidInput(handler: (inputValue: string) => void) {
    this._element.addEventListener("input", () => {
      this.validateAndUpdate(handler);
    });
  }
  private validateAndUpdate(handler: (inputValue: string) => void) {
    const value: string = this._element.value;
    const isValid: boolean = isPositiveNumber(value);
    if (isValid) {
      this._element.className = classNameValidInput;
      handler(value);
    } else {
      this._element.className = classNameInvalidInput;
    }
  }
}

import { getElementByIdOrThrow } from "./getElement";

export class InputRange {
  private elementLabel: string;
  private inputElement: HTMLInputElement;
  private labelElement: HTMLLabelElement;

  public constructor(elementLabel: string) {
    this.elementLabel = elementLabel;
    this.inputElement = getElementByIdOrThrow(HTMLInputElement, `${elementLabel}-input`);
    this.labelElement = getElementByIdOrThrow(HTMLLabelElement, `${elementLabel}-label`);
  }

  public onWindowLoad(handler: (inputValue: string) => void) {
    this.update(handler);
  }

  public onInput(handler: (inputValue: string) => void) {
    this.inputElement.addEventListener("input", () => {
      this.update(handler);
    });
  }

  private update(handler: (inputValue: string) => void) {
    const value: string = this.inputElement.value;
    this.labelElement.textContent = `${this.elementLabel}:${value}`;
    handler(value);
  }
}

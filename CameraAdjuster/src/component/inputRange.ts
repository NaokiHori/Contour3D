import { getElementByIdUnwrap } from "./getElement";

export class InputRange {
  private _element_label: string;
  private _input_element: HTMLInputElement;
  private _label_element: HTMLLabelElement;
  public constructor(elementLabel: string) {
    this._element_label = elementLabel;
    this._input_element = getElementByIdUnwrap(
      `${elementLabel}-input`,
    ) as HTMLInputElement;
    this._label_element = getElementByIdUnwrap(
      `${elementLabel}-label`,
    ) as HTMLLabelElement;
  }
  public onWindowLoad(handler: (inputValue: string) => void) {
    this.update(handler);
  }
  public onInput(handler: (inputValue: string) => void) {
    this._input_element.addEventListener("input", () => {
      this.update(handler);
    });
  }
  private update(handler: (inputValue: string) => void) {
    const value: string = this._input_element.value;
    this._label_element.textContent = `${this._element_label}:${value}`;
    handler(value);
  }
}

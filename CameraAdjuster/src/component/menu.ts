import { config } from "../config";
import { getElementByIdOrThrow } from "./getElement";
import { syncCanvasSize, updateCanvas } from "./canvas";

const menuItems = [
  getElementByIdOrThrow(HTMLDivElement, "screen-size"),
  getElementByIdOrThrow(HTMLDivElement, "distance"),
  getElementByIdOrThrow(HTMLDivElement, "box-size"),
  getElementByIdOrThrow(HTMLDivElement, "angle"),
];
const numberOfItems = menuItems.length;

let selectedIndex = numberOfItems - 1;

function updateMenuItem() {
  for (let n = 0; n < numberOfItems; n++) {
    menuItems[n].style.display = n === selectedIndex ? "flex" : "none";
  }
}

function goToLeft() {
  selectedIndex = (selectedIndex + numberOfItems - 1) % numberOfItems;
  updateMenuItem();
  syncCanvasSize();
  updateCanvas(config);
}

function goToRight() {
  selectedIndex = (selectedIndex + numberOfItems + 1) % numberOfItems;
  updateMenuItem();
  syncCanvasSize();
  updateCanvas(config);
}

const buttons = {
  left: getElementByIdOrThrow(HTMLButtonElement, "menu-go-to-left"),
  right: getElementByIdOrThrow(HTMLButtonElement, "menu-go-to-right"),
};

export function initMenuItems() {
  buttons.left.addEventListener("click", goToLeft);
  buttons.right.addEventListener("click", goToRight);
}

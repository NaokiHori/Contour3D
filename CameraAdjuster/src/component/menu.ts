import { config } from "../config";
import { getElementByIdUnwrap } from "./getElement";
import { syncCanvasSize, updateCanvas } from "./canvas";

const menuItems = [
  getElementByIdUnwrap("screen-size") as HTMLDivElement,
  getElementByIdUnwrap("distance") as HTMLDivElement,
  getElementByIdUnwrap("box-size") as HTMLDivElement,
  getElementByIdUnwrap("angle") as HTMLDivElement,
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
  left: getElementByIdUnwrap("menu-go-to-left") as HTMLButtonElement,
  right: getElementByIdUnwrap("menu-go-to-right") as HTMLButtonElement,
};

export function initMenuItems() {
  buttons.left.addEventListener("click", goToLeft);
  buttons.right.addEventListener("click", goToRight);
}

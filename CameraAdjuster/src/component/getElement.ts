export function getElementByIdUnwrap(id: string): HTMLElement {
  const elem: HTMLElement | null = document.getElementById(id);
  if (elem === null) {
    throw new Error(`failed to get element: ${id}`);
  }
  return elem;
}

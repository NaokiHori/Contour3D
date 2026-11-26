export function getElementByIdOrThrow<T extends Element>(
  type: new (...args: unknown[]) => T,
  id: string,
): T {
  const element = document.querySelector(`#${id}`);
  if (null === element) {
    throw new Error(`element not found: #${id}`);
  }
  if (!(element instanceof type)) {
    throw new TypeError(`unexpected type of element: #${id}`);
  }
  return element;
}

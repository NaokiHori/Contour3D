export const classNameValidInput = "valid-value";
export const classNameInvalidInput = "invalid-value";

export function isPositiveNumber(str: string): boolean {
  const num = Number(str);
  if (Number.isNaN(num)) {
    return false;
  }
  if (num <= 0) {
    return false;
  }
  return true;
}

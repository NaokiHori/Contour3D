export function drawCaption(ctx: CanvasRenderingContext2D) {
  ctx.font = "36px Arial";
  ctx.fillStyle = "#ff00ff";
  ctx.fillText("- x", 20, 30);
  ctx.fillStyle = "#ffd700";
  ctx.fillText("- y", 20, 60);
  ctx.fillStyle = "#00ffff";
  ctx.fillText("- z", 20, 90);
}

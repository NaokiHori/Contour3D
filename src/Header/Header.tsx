import { JSX } from "react";
import * as style from "./style.css";

export function Header(): JSX.Element {
  const href = "https://github.com/NaokiHori/Contour3D";
  return (
    <div className={style.container}>
      <a className={style.text} href={href}>
        Contour3D
      </a>
    </div>
  );
}

import { JSX} from "react";
import * as style from "./style.css"

export function Canvas(): JSX.Element {
  return (
    <div className={style.main}>
      This is my canvas element
    </div>
    );
}

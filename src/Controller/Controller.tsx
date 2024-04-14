import { JSX} from "react";
import * as style from "./style.css"

export function Controller(): JSX.Element {
  return (
    <div className={style.main}>
      This is my controller
    </div>
    );
}

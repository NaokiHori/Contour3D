import { JSX } from "react";
import { Header } from "./Header/Header";
import { Canvas } from "./Canvas/Canvas";
import { Controller } from "./Controller/Controller";
import * as style from "./style.css";

export function App(): JSX.Element {
  return (
    <div className={style.app}>
      <Header />
      <div className={style.main}>
        <Canvas />
        <Controller />
      </div>
    </div>
  );
}

import tseslint from "typescript-eslint";

export default tseslint.config(
  ...tseslint.configs.strictTypeChecked,
  ...tseslint.configs.stylisticTypeChecked,
  {
    languageOptions: {
      parserOptions: {
        project: true,
        tsconfigRootDir: import.meta.dirname,
      },
    },
  },
  {
    rules: {
      "@typescript-eslint/switch-exhaustiveness-check": "error",
      "@typescript-eslint/array-type": [
        "error",
        {
          default: "generic",
        },
      ],
    },
  },
  {
    ignores: ["node_modules/*", "dist/*", "eslint.config.js", "vite.config.ts"],
  },
);

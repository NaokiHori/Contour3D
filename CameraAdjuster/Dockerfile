FROM node:alpine

RUN mkdir /app && chown node:node /app
USER node
WORKDIR /app

COPY --chown=node:node tools/package.json tools/package-lock.json .
RUN npm ci --ignore-scripts

COPY --chown=node:node tools/.oxlintrc.json tools/tsconfig.json tools/vite.config.ts .

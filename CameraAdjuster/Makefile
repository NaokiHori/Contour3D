IMAGE := naokihori/contour-3d-camera-adjuster:latest
CONTAINER := contour-3d-camera-adjuster
ARTIFACT ?= dist
BASE ?= ./

help:
	@echo "build       : build artifacts"
	@echo "build-image : build docker image"
	@echo "dev         : launch development server"
	@echo "polish      : run type check, formatter, and linter"
	@echo "test        : run test"

build:
	docker rm -f $(CONTAINER) 2>/dev/null || true
	docker run \
    --name $(CONTAINER) \
    -v "${PWD}/index.html":/app/index.html:ro \
    -v "${PWD}/src":/app/src:ro \
    -v "${PWD}/style":/app/style:ro \
    $(IMAGE) \
    npx vite build --outDir=/app/dist --base=$(BASE)
	docker cp $(CONTAINER):/app/dist $(ARTIFACT)
	docker rm $(CONTAINER)

build-image:
	docker build --progress=plain -t $(IMAGE) .

dev:
	docker run \
    --rm \
    -p 5173:5173 \
    --name $(CONTAINER) \
    -v "${PWD}/index.html":/app/index.html:ro \
    -v "${PWD}/src":/app/src:ro \
    -v "${PWD}/style":/app/style:ro \
    $(IMAGE) \
    npx vite dev --host

polish:
	# tsc, all sources are needed to type-check
	docker run \
    --rm \
    --name $(CONTAINER) \
    -v "${PWD}/src":/app/src:ro \
    $(IMAGE) \
    npx tsc
	# formatter, writable access is needed to fix format violation
	docker run \
    --rm \
    --name $(CONTAINER) \
    -v "${PWD}/index.html":/app/index.html \
    -v "${PWD}/src":/app/src \
    -v "${PWD}/style":/app/style \
    $(IMAGE) \
    npx oxfmt
	# linter, all sources are needed to type-check
	docker run \
    --rm \
    --name $(CONTAINER) \
    -v "${PWD}/src":/app/src:ro \
    $(IMAGE) \
    npx oxlint --type-aware src/


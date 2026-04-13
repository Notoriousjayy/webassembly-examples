import createModule from "./testProject.js";

const canvas = document.getElementById("canvas");
const status = document.getElementById("status");

function setStatus(message) {
  if (status) {
    status.textContent = message;
  }
}

function resizeCanvas() {
  if (!canvas) return;

  const dpr = globalThis.devicePixelRatio || 1;
  const cssWidth = Math.max(1, canvas.clientWidth || globalThis.innerWidth || 1280);
  const cssHeight = Math.max(1, canvas.clientHeight || globalThis.innerHeight || 720);

  canvas.width = Math.max(1, Math.floor(cssWidth * dpr));
  canvas.height = Math.max(1, Math.floor(cssHeight * dpr));
}

resizeCanvas();
globalThis.addEventListener("resize", resizeCanvas);

try {
  setStatus("Loading ES-module WebAssembly runtime…");

  const Module = await createModule({
    canvas,
    noInitialRun: true,
    print: (text) => console.log(text),
    printErr: (text) => console.error(text)
  });

  globalThis.testProjectModule = Module;

  setStatus("Initializing SDL3 + WebGL2 renderer…");

  const ok = Module.ccall("initWebGL", "number", [], []);
  if (!ok) {
    throw new Error("initWebGL returned failure");
  }

  setStatus("Running");

  try {
    Module.ccall("startMainLoop", null, [], []);
  } catch (error) {
    if (error !== "unwind") {
      throw error;
    }
  }
} catch (error) {
  console.error("Bootstrap failure:", error);
  setStatus(`Startup failed: ${error instanceof Error ? error.message : String(error)}`);
}

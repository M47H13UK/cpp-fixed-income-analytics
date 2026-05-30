import { defineConfig } from 'vite';
import react from '@vitejs/plugin-react';

// GitHub Pages serves this project site from /<repo>/, so the base path is the
// repo name. It is set unconditionally (dev, preview and build) so that
// import.meta.env.BASE_URL is a consistent ABSOLUTE prefix everywhere - the
// WASM loader joins it with `engine/engine.mjs` to fetch the module, and a
// relative base would resolve that against the JS chunk instead of the site root.
export default defineConfig({
  base: '/cpp-fixed-income-analytics/',
  plugins: [react()],
});

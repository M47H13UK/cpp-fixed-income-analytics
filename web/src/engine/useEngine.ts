import { useEffect, useState } from 'react';
import type { Engine, EngineFactory } from './types';

// Loads the WASM engine once. engine.mjs and engine.wasm live in /public/engine
// and are fetched from BASE_URL so the same code works at dev root and under the
// GitHub Pages sub-path. The /* @vite-ignore */ keeps Vite from trying to bundle
// emscripten's runtime glue - we want it loaded as-is at runtime.
export function useEngine(): { engine: Engine | null; error: string | null } {
  const [engine, setEngine] = useState<Engine | null>(null);
  const [error, setError] = useState<string | null>(null);

  useEffect(() => {
    let alive = true;
    const base = import.meta.env.BASE_URL;
    import(/* @vite-ignore */ `${base}engine/engine.mjs`)
      .then((mod) => {
        const create = mod.default as EngineFactory;
        return create({ locateFile: (path: string) => `${base}engine/${path}` });
      })
      .then((instance) => {
        if (alive) setEngine(instance);
      })
      .catch((e) => {
        if (alive) setError(e instanceof Error ? e.message : String(e));
      });
    return () => {
      alive = false;
    };
  }, []);

  return { engine, error };
}

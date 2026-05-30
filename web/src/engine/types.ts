// TypeScript view of the embind API exported from wasm/bindings.cpp.
// All rates are in PERCENT across this boundary; dayCount is the DayCount enum
// index (0=30/360, 1=ACT/360, 2=ACT/365F, 3=ACT/ACT).

export interface Analytics {
  dirty: number;
  clean: number;
  accrued: number;
  macaulay: number;
  modified: number;
  convexity: number;
  dv01: number;
  ytmCheck: number;
  periods: number;
}

export interface PricePoint {
  yield: number;
  price: number;
}

export interface Cashflow {
  year: number;
  amount: number;
}

export interface CurvePoint {
  year: number;
  par: number;
  zero: number;
  forward: number;
}

export interface Engine {
  analyze(
    face: number, couponPct: number, freq: number, years: number,
    ytmPct: number, dayCount: number,
  ): Analytics;

  priceYieldCurve(
    face: number, couponPct: number, freq: number, years: number,
    yLoPct: number, yHiPct: number, n: number, dayCount: number,
  ): PricePoint[];

  cashflows(
    face: number, couponPct: number, freq: number, years: number, dayCount: number,
  ): Cashflow[];

  yieldToMaturity(
    face: number, couponPct: number, freq: number, years: number,
    cleanPrice: number, dayCount: number,
  ): number;

  bootstrapCurve(parYieldsPct: number[]): CurvePoint[];
}

// Factory shape produced by emscripten's MODULARIZE + EXPORT_ES6 build.
export type EngineFactory = (opts?: {
  locateFile?: (path: string) => string;
}) => Promise<Engine>;

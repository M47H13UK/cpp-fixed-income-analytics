// UI-side bond inputs and the option lists shared by the form.

export interface BondInputs {
  face: number;
  couponPct: number;
  ytmPct: number;
  years: number;
  freq: number;     // coupons per year: 1, 2, 4, 12
  dayCount: number; // DayCount enum index: 0=30/360 1=ACT/360 2=ACT/365F 3=ACT/ACT
}

export const DEFAULTS: BondInputs = {
  face: 1000,
  couponPct: 5,
  ytmPct: 6,
  years: 5,
  freq: 2,
  dayCount: 0,
};

export const FREQUENCIES = [
  { v: 1, label: 'Annual' },
  { v: 2, label: 'Semi-annual' },
  { v: 4, label: 'Quarterly' },
  { v: 12, label: 'Monthly' },
];

export const DAY_COUNTS = [
  { v: 0, label: '30/360' },
  { v: 1, label: 'ACT/360' },
  { v: 2, label: 'ACT/365F' },
  { v: 3, label: 'ACT/ACT' },
];

export const fmt = (n: number, d = 2): string =>
  n.toLocaleString('en-US', { minimumFractionDigits: d, maximumFractionDigits: d });

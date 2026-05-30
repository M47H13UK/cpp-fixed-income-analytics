import type { BondInputs } from '../model';
import { DAY_COUNTS, FREQUENCIES } from '../model';

interface Props {
  inputs: BondInputs;
  onChange: (patch: Partial<BondInputs>) => void;
}

function NumberField({
  label, value, onChange, step = 1, min = 0, suffix,
}: {
  label: string; value: number; onChange: (v: number) => void;
  step?: number; min?: number; suffix?: string;
}) {
  return (
    <label className="field">
      <span className="field-label">{label}</span>
      <span className="field-input">
        <input
          type="number"
          value={Number.isFinite(value) ? value : ''}
          step={step}
          min={min}
          onChange={(e) => onChange(parseFloat(e.target.value))}
        />
        {suffix && <span className="field-suffix">{suffix}</span>}
      </span>
    </label>
  );
}

export function BondForm({ inputs, onChange }: Props) {
  return (
    <section className="panel form-panel">
      <h2 className="panel-title">Bond terms</h2>

      <NumberField label="Face value" value={inputs.face} step={100}
                   onChange={(v) => onChange({ face: v })} suffix="$" />
      <NumberField label="Coupon rate" value={inputs.couponPct} step={0.25}
                   onChange={(v) => onChange({ couponPct: v })} suffix="%" />
      <NumberField label="Market yield (YTM)" value={inputs.ytmPct} step={0.25}
                   onChange={(v) => onChange({ ytmPct: v })} suffix="%" />
      <NumberField label="Years to maturity" value={inputs.years} step={1} min={1}
                   onChange={(v) => onChange({ years: Math.max(1, Math.round(v)) })} suffix="yr" />

      <label className="field">
        <span className="field-label">Coupon frequency</span>
        <select value={inputs.freq} onChange={(e) => onChange({ freq: Number(e.target.value) })}>
          {FREQUENCIES.map((f) => <option key={f.v} value={f.v}>{f.label}</option>)}
        </select>
      </label>

      <label className="field">
        <span className="field-label">Day-count convention</span>
        <select value={inputs.dayCount} onChange={(e) => onChange({ dayCount: Number(e.target.value) })}>
          {DAY_COUNTS.map((d) => <option key={d.v} value={d.v}>{d.label}</option>)}
        </select>
      </label>
    </section>
  );
}

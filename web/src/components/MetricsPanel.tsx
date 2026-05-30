import type { Analytics } from '../engine/types';
import { fmt } from '../model';

interface Props {
  analytics: Analytics;
  face: number;
}

function Metric({ label, value, hint }: { label: string; value: string; hint?: string }) {
  return (
    <div className="metric">
      <div className="metric-label">{label}</div>
      <div className="metric-value">{value}</div>
      {hint && <div className="metric-hint">{hint}</div>}
    </div>
  );
}

export function MetricsPanel({ analytics: a, face }: Props) {
  const premium = a.clean > face;
  const par = Math.abs(a.clean - face) < 0.005;
  const tag = par ? 'at par' : premium ? 'premium' : 'discount';

  return (
    <section className="panel metrics-panel">
      <h2 className="panel-title">Analytics</h2>

      <div className="hero">
        <div className="hero-label">Clean price</div>
        <div className="hero-value">{fmt(a.clean)}</div>
        <div className={`hero-tag tag-${tag.replace(' ', '-')}`}>{tag}</div>
      </div>

      <div className="metric-grid">
        <Metric label="Dirty price" value={fmt(a.dirty)} />
        <Metric label="Accrued interest" value={fmt(a.accrued)} />
        <Metric label="Macaulay duration" value={fmt(a.macaulay)} hint="years" />
        <Metric label="Modified duration" value={fmt(a.modified)} hint="per 100% yield" />
        <Metric label="Convexity" value={fmt(a.convexity)} />
        <Metric label="DV01" value={fmt(a.dv01, 4)} hint="per 1bp" />
        <Metric label="Cash flows" value={String(a.periods)} />
        <Metric label="YTM (recovered)" value={`${fmt(a.ytmCheck)}%`} hint="price → yield solve" />
      </div>
    </section>
  );
}

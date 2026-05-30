import {
  CartesianGrid, Line, LineChart, ReferenceDot, ResponsiveContainer, Tooltip, XAxis, YAxis,
} from 'recharts';
import type { PricePoint } from '../engine/types';
import { fmt } from '../model';

interface Props {
  data: PricePoint[];
  currentYield: number;
  currentPrice: number;
}

const tooltipStyle = {
  background: '#0d1320',
  border: '1px solid #243044',
  borderRadius: 8,
  fontSize: 12,
};

export function PriceYieldChart({ data, currentYield, currentPrice }: Props) {
  return (
    <section className="panel chart-panel">
      <h2 className="panel-title">Price / yield curve</h2>
      <p className="chart-caption">
        Convex relationship between market yield and clean price. The marker is the
        current yield.
      </p>
      <ResponsiveContainer width="100%" height={280}>
        <LineChart data={data} margin={{ top: 8, right: 20, bottom: 8, left: 4 }}>
          <CartesianGrid stroke="#1b2533" strokeDasharray="3 3" />
          <XAxis
            dataKey="yield" type="number" domain={['dataMin', 'dataMax']}
            tickFormatter={(v: number) => `${v.toFixed(0)}%`}
            stroke="#6b7a90" fontSize={12} tickMargin={8}
          />
          <YAxis
            tickFormatter={(v: number) => v.toFixed(0)}
            stroke="#6b7a90" fontSize={12} width={52}
            domain={['auto', 'auto']}
          />
          <Tooltip
            contentStyle={tooltipStyle}
            labelFormatter={(v: number) => `Yield ${fmt(v)}%`}
            formatter={(value: any) => [fmt(value), 'Clean price']}
          />
          <Line type="monotone" dataKey="price" stroke="#5b8cff" strokeWidth={2.2} dot={false} />
          <ReferenceDot
            x={currentYield} y={currentPrice} r={5}
            fill="#f0b429" stroke="#0b0f17" strokeWidth={2} isFront
          />
        </LineChart>
      </ResponsiveContainer>
    </section>
  );
}

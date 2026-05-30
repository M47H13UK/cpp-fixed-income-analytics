import {
  Bar, BarChart, CartesianGrid, ResponsiveContainer, Tooltip, XAxis, YAxis,
} from 'recharts';
import type { Cashflow } from '../engine/types';
import { fmt } from '../model';

interface Props {
  data: Cashflow[];
}

const tooltipStyle = {
  background: '#0d1320',
  border: '1px solid #243044',
  borderRadius: 8,
  fontSize: 12,
};

export function CashflowChart({ data }: Props) {
  return (
    <section className="panel chart-panel">
      <h2 className="panel-title">Cash-flow schedule</h2>
      <p className="chart-caption">
        Coupons over the bond's life, with the face value repaid at maturity.
      </p>
      <ResponsiveContainer width="100%" height={280}>
        <BarChart data={data} margin={{ top: 8, right: 20, bottom: 8, left: 4 }}>
          <CartesianGrid stroke="#1b2533" strokeDasharray="3 3" vertical={false} />
          <XAxis
            dataKey="year"
            tickFormatter={(v: number) => `${fmt(v, 1)}y`}
            stroke="#6b7a90" fontSize={12} tickMargin={8}
          />
          <YAxis stroke="#6b7a90" fontSize={12} width={52} />
          <Tooltip
            contentStyle={tooltipStyle}
            labelFormatter={(v: number) => `Year ${fmt(v, 2)}`}
            formatter={(value: any) => [fmt(value), 'Cash flow']}
            cursor={{ fill: '#16202e' }}
          />
          <Bar dataKey="amount" fill="#2dd4a7" radius={[3, 3, 0, 0]} />
        </BarChart>
      </ResponsiveContainer>
    </section>
  );
}

"use client";

import useChart from "../hooks/useChart";

import { Button } from "@/components/ui/button";
import {
  ChartConfig,
  ChartContainer,
  ChartLegend,
  ChartLegendContent,
  ChartTooltip,
  ChartTooltipContent,
} from "@/components/ui/chart";
import { Brush, CartesianGrid, Line, LineChart, XAxis, YAxis } from "recharts";
import ParameterController from "./parameter-controller";

const chartConfig = {
  angle: {
    label: "Angle",
    color: "var(--chart-1)",
  },
  input: {
    label: "Voltage",
    color: "var(--chart-2)",
  },
  target: {
    label: "Target",
    color: "var(--chart-3)",
  },
} satisfies ChartConfig;

export const Chart = () => {
  const { connect, close, isConnected, data } = useChart({
    defaultUrl: process.env.NEXT_PUBLIC_WS_URL + "/ws",
    defaultRetention: 100,
  });

  return (
    <div>
      <div className="flex justify-end space-x-4">
        <ParameterController />
        {isConnected ? (
          <Button
            className="w-32"
            variant={"destructive"}
            disabled={!isConnected}
            onClick={() => {
              close();
            }}
          >
            close
          </Button>
        ) : (
          <Button
            className="w-32"
            disabled={isConnected}
            onClick={() => {
              connect();
            }}
          >
            connect
          </Button>
        )}
      </div>
      <ChartContainer
        className="w-full min-h-[200px] mt-4 max-h-[640px]"
        config={chartConfig}
      >
        <LineChart accessibilityLayer data={data} height={200}>
          <XAxis dataKey="time" domain={["auto", "auto"]} />
          <CartesianGrid vertical={false} />
          <ChartLegend content={<ChartLegendContent />} verticalAlign="top" />
          <YAxis yAxisId="angle" domain={[70, 110]} />
          <YAxis yAxisId="voltage" domain={[-5, 5]} orientation="right" />
          <Line
            yAxisId="angle"
            dataKey={"angle"}
            type={"linear"}
            strokeWidth={1}
            stroke="var(--color-angle)"
            dot={false}
            isAnimationActive={false}
          />
          <Line
            yAxisId="angle"
            dataKey={"target"}
            type={"linear"}
            strokeWidth={1}
            stroke="var(--color-target)"
            dot={false}
            isAnimationActive={false}
          />
          <Line
            yAxisId="voltage"
            dataKey={"input"}
            type={"linear"}
            strokeWidth={1}
            stroke="var(--color-input)"
            dot={false}
            isAnimationActive={false}
          />
          <Brush
            dataKey={"time"}
            height={30}
            fill="var(--background)"
            stroke="var(--primary)"
          >
            <LineChart data={data}>
              <XAxis dataKey="time" domain={["auto", "auto"]} hide />
              <YAxis yAxisId="angle" domain={[70, 110]} hide />
              <Line
                yAxisId="angle"
                dataKey={"angle"}
                type={"linear"}
                strokeWidth={1}
                stroke="var(--color-angle)"
                dot={false}
                isAnimationActive={false}
              />
              <Line
                yAxisId="angle"
                dataKey={"target"}
                type={"linear"}
                strokeWidth={1}
                stroke="var(--color-target)"
                dot={false}
                isAnimationActive={false}
              />
            </LineChart>
          </Brush>
          <ChartTooltip cursor={false} content={<ChartTooltipContent />} />
        </LineChart>
      </ChartContainer>
    </div>
  );
};

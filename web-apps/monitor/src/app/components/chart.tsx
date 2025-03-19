"use client";

import { ChartData, ChartOptions, TimeScale } from "chart.js";
import "chartjs-adapter-moment";
import { Line } from "react-chartjs-2";
import useChart from "../hooks/useChart";

import { Button } from "@/components/ui/button";
import { Input } from "@/components/ui/input";
import {
  CategoryScale,
  Chart as ChartJS,
  Filler,
  Legend,
  LinearScale,
  LineElement,
  PointElement,
  Title,
  Tooltip,
} from "chart.js";

ChartJS.register(
  CategoryScale,
  LinearScale,
  TimeScale,
  PointElement,
  LineElement,
  Title,
  Tooltip,
  Legend,
  Filler
);

const initData: ChartData<"line", number[], number> = {
  labels: [],
  datasets: [
    {
      label: "target angle",
      type: "line",
      data: [],
      spanGaps: true,
      fill: false,
      pointStyle: false,
      borderJoinStyle: "bevel",
      yAxisID: "angle",
    },
    {
      label: "current angle",
      type: "line",
      spanGaps: true,
      pointStyle: false,
      data: [],
      borderColor: "rgba(75, 192, 192, 1)",
      fill: false,
      yAxisID: "angle",
    },
    {
      label: "input voltage",
      type: "line",
      spanGaps: true,
      pointStyle: false,
      data: [],
      borderColor: "red",
      fill: false,
      yAxisID: "voltage",
    },
  ],
};

const options: ChartOptions<"line"> = {
  animation: false,
  responsive: true,
  plugins: {
    legend: {
      position: "top",
    },
    title: {
      display: false,
    },
    tooltip: {
      enabled: false,
    },
  },
  scales: {
    x: { type: "time", ticks: { autoSkip: true } },
    angle: {
      position: "left",
      suggestedMax: 110,
      suggestedMin: 70,
      title: { display: true, text: "angle" },
    },
    voltage: {
      position: "right",
      suggestedMin: -5,
      suggestedMax: 5,
      title: { display: true, text: "voltage" },
    },
  },
};

export const Chart = () => {
  const { connect, close, url, setUrl, chartRef } = useChart({
    defaultUrl: process.env.NEXT_PUBLIC_WS_URL + "/ws",
    initData: initData,
  });

  return (
    <div className="space-y-4">
      <div className="flex justify-end space-x-4">
        <Input
          className="w-96"
          onChange={(e) => setUrl(e.target.value)}
          placeholder="input url"
          value={url}
        />
        <Button
          onClick={() => {
            connect();
          }}
        >
          connect
        </Button>
        <Button
          variant={"destructive"}
          onClick={() => {
            close();
          }}
        >
          close
        </Button>
      </div>

      <div>
        <Line ref={chartRef} options={options} data={initData} />;
      </div>
    </div>
  );
};

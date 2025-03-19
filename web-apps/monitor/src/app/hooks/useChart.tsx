"use client";
import ControlData from "@/libs/types/controlData";
import { ChartData, Chart as ChartJS } from "chart.js";
import { useCallback, useEffect, useRef, useState } from "react";
import useWebSocket from "./useWebSocket";

const useChart = ({
  defaultRetention,
  defaultUrl,
  initData,
}: {
  defaultRetention?: number;
  defaultUrl?: string;
  initData: ChartData<"line", number[], number>;
}) => {
  const {
    receive,
    connect: connectWs,
    close: closeWs,
    isConnected,
  } = useWebSocket<ControlData>();
  const [url, setUrl] = useState<string | undefined>(defaultUrl);
  const [retention, setRetention] = useState<number>(defaultRetention ?? 100);

  const chartRef = useRef<ChartJS<"line", number[], number>>(undefined);

  const [data, setData] =
    useState<ChartData<"line", number[], number>>(initData);

  const updateData = useCallback(
    (message: ControlData) => {
      setData((prev) => {
        const { target, angle, time, input } = message;
        const newLabels = [...(prev.labels ?? []), time];
        const newTarget = [...(prev.datasets[0]?.data ?? []), target];
        const newAngle = [...(prev.datasets[1]?.data ?? []), angle];
        const newVolt = [...(prev.datasets[2]?.data ?? []), input];
        return {
          labels: newLabels.slice(-retention),
          datasets: [
            {
              ...prev.datasets[0],
              data: newTarget.slice(-retention),
            },
            {
              ...prev.datasets[1],
              data: newAngle.slice(-retention),
            },
            {
              ...prev.datasets[2],
              data: newVolt.slice(-retention),
            },
          ],
        };
      });
    },
    [retention]
  );

  const clearData = useCallback(() => {
    setData((prev) => {
      return {
        labels: [],
        datasets: [
          {
            ...prev.datasets[0],
            data: [],
          },
          {
            ...prev.datasets[1],
            data: [],
          },
          {
            ...prev.datasets[2],
            data: [],
          },
        ],
      };
    });
  }, []);

  const close = useCallback(() => {
    closeWs();
    clearData();
  }, [closeWs, clearData]);

  const connect = useCallback(() => {
    if (url) {
      connectWs(url);
    }
  }, [connectWs, url]);

  useEffect(() => {
    const chart = chartRef.current;
    if (!chart) {
      return;
    }
    chart.data = data;
    chart.update();
  }, [chartRef, data]);

  useEffect(() => {
    let isMounted = true;
    const polling = async () => {
      const message = await receive();
      updateData(message);
    };
    const run = () => {
      polling().finally(() => {
        if (isMounted) {
          setTimeout(run);
        }
      });
    };
    run();
    return () => {
      isMounted = false;
    };
  }, [receive, updateData]);

  return {
    connect,
    close,
    url,
    setUrl,
    setRetention,
    chartRef,
    isConnected,
  };
};

export default useChart;

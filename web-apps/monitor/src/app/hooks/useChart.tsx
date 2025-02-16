"use client";
import ControlData from "@/libs/types/controlData";
import { ChartData } from "chart.js";
import { useCallback, useEffect, useState } from "react";
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
  } = useWebSocket<ControlData>();
  const [url, setUrl] = useState<string>(defaultUrl ?? "");
  const [retention, setRetention] = useState<number>(defaultRetention ?? 500);

  const [data, setData] =
    useState<ChartData<"line", number[], number>>(initData);

  const updateData = useCallback(
    (message: ControlData) => {
      setData((prev) => {
        const { target, angle } = message;
        const newLabels = [...(prev.labels ?? []), new Date().getTime()];
        const newTarget = [...(prev.datasets[0].data ?? []), target];
        const newAngle = [...(prev.datasets[1].data ?? []), angle];
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
        ],
      };
    });
  }, []);

  const close = useCallback(() => {
    closeWs();
    clearData();
  }, [closeWs, clearData]);

  const connect = useCallback(() => {
    connectWs(url);
  }, [connectWs, url]);

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
    data,
    url,
    setUrl,
    setRetention,
  };
};

export default useChart;

"use client";
import ControlData from "@/libs/types/controlData";
import { useCallback, useEffect, useState } from "react";
import useWebSocket from "./useWebSocket";

const useChart = ({
  defaultRetention,
  defaultUrl,
}: {
  defaultRetention?: number;
  defaultUrl?: string;
}) => {
  const {
    receive,
    connect: connectWs,
    close: closeWs,
  } = useWebSocket<ControlData>();
  const [url, setUrl] = useState<string | undefined>(defaultUrl);
  const [retention, setRetention] = useState<number>(defaultRetention ?? 100);

  const [data, setData] = useState<ControlData[]>([]);
  const [isConnected, setIsConnected] = useState<boolean>(false);

  const updateData = useCallback(
    (message: ControlData) => {
      setData((prev) => {
        return [...prev, message].slice(-retention);
      });
    },
    [retention]
  );

  const clearData = useCallback(() => {
    setData([]);
  }, []);

  const close = useCallback(() => {
    setIsConnected(false);
    closeWs();
  }, [closeWs, clearData, setIsConnected]);

  const connect = useCallback(() => {
    if (url) {
      clearData();
      setIsConnected(true);
      connectWs(url);
    }
  }, [url, setIsConnected, connectWs]);

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
    isConnected,
    data,
  };
};

export default useChart;

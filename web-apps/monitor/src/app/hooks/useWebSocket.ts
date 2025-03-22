"use client";
import { useCallback, useRef } from "react";
import { toast } from "sonner";
import { PromiseQueue } from "../../libs/types/promiseQueue";

const useWebSocket = <T>() => {
  const socketRef = useRef<WebSocket>(undefined);
  const queue = useRef<PromiseQueue<T>>(new PromiseQueue());

  const close = useCallback(() => {
    if (!socketRef.current) {
      toast.error("already closed");
      return;
    }
    socketRef?.current?.close();
    socketRef.current = undefined;
    queue.current = new PromiseQueue();
  }, []);

  const connect = useCallback(
    (url: string) => {
      if (socketRef.current) {
        toast.error("already connected");
        return;
      }

      const ws = new WebSocket(url);
      ws.onmessage = (event) => {
        queue.current.send(JSON.parse(event.data));
      };
      ws.onerror = (event) => {
        console.error(event);
        toast.error("an error occurred. please check the console");
        close();
      };
      socketRef.current = ws;
    },
    [close]
  );

  return {
    receive: queue.current.receive,
    connect,
    close,
  };
};

export default useWebSocket;

"use client";
import { useCallback, useRef } from "react";
import { PromiseQueue } from "../../libs/types/promiseQueue";

const useWebSocket = <T>() => {
  const socketRef = useRef<WebSocket>(undefined);
  const queue = useRef<PromiseQueue<T>>(new PromiseQueue());

  const close = useCallback(() => {
    if (!socketRef.current) {
      alert("already closed");
      return;
    }
    socketRef?.current?.close();
    socketRef.current = undefined;
    queue.current = new PromiseQueue();
  }, []);

  const connect = useCallback(
    (url: string) => {
      if (socketRef.current) {
        alert("already connected");
        return;
      }

      const ws = new WebSocket(url);
      ws.onmessage = (event) => {
        queue.current.send(JSON.parse(event.data));
      };
      ws.onerror = (event) => {
        console.error(event);
        alert("an error occurred. please check the console");
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

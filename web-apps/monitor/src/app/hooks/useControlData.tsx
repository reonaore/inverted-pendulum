"use client";

import { createContext, ReactNode, useContext } from "react";

export interface ControlData {
  ki: number;
  kd: number;
  kp: number;
  target: number;
}

export interface ControlDataContextType {
  promise: Promise<ControlData | undefined>;
}

const ControlDataContext = createContext<ControlDataContextType | undefined>(
  undefined
);

export const useControlData = () => {
  const context = useContext(ControlDataContext);
  if (!context) {
    throw new Error("must be used within a Provider");
  }
  return context;
};

export const ControlDataProvider = ({
  children,
  parameterPromise: dataPromise,
}: {
  children: ReactNode;
  parameterPromise: Promise<ControlData | undefined>;
}) => {
  return (
    <ControlDataContext.Provider value={{ promise: dataPromise }}>
      {children}
    </ControlDataContext.Provider>
  );
};

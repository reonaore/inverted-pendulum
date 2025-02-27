"use server";

import { ControlData } from "@/app/hooks/useControlData";
import { ControllerParameters } from "@/libs/types/controlParameter";

const basePath = process.env.NEXT_PUBLIC_API_URL || "";

export const getParameters = async () => {
  const response = await fetch(basePath + "/controller/parameters");
  return (await response.json()) as ControllerParameters;
};

export const updateParameters = async (params: ControllerParameters) => {
  const res = await fetch(basePath + "/controller/parameters", {
    method: "POST",
    headers: {
      "Content-Type": "application/json",
    },
    body: JSON.stringify(params),
  });
  if (!res.ok) {
    throw new Error("unable to update");
  }
};

export const getTargetAngle = async () => {
  const response = await fetch(basePath + "/controller/target-angle");
  return (await response.json()) as { angle: number };
};

export const updateTargetAngle = async (target: number) => {
  const res = await fetch(basePath + "/controller/target-angle", {
    method: "POST",
    headers: {
      "Content-Type": "application/json",
    },
    body: JSON.stringify({ angle: target }),
  });
  if (!res.ok) {
    throw new Error("unable to update");
  }
};

export const getControlData = async () => {
  const params = await getParameters();
  const target = await getTargetAngle();
  return {
    ki: params.ki,
    kd: params.kd,
    kp: params.kp,
    target: target.angle,
  };
};

export const updateControlData = async (data: ControlData) => {
  await updateParameters({ ...data });
  await updateTargetAngle(data.target);
};

# rwthacs/dpsim-dev is built by dpsim-git/Packaging/Docker/Dockerfile.dev
ARG DOCKER_TAG=latest
ARG CI
ARG CI_COMMIT_SHA
ARG CI_COMMIT_REF_NAME
ARG CI_COMMIT_TAG

FROM rwthacs/dpsim-dev:${DOCKER_TAG} AS builder

COPY . /dpsim/

RUN rm -rf /dpsim/build && mkdir /dpsim/build 
WORKDIR /dpsim/build

RUN cmake -DBUILD_EXAMPLES=OFF -DCPACK_GENERATOR=RPM ..
RUN make -j$(nproc) package

FROM fedora:29

LABEL \
	org.label-schema.schema-version = "1.0" \
	org.label-schema.name = "DPsim" \
	org.label-schema.license = "GPL-3.0" \
	org.label-schema.vendor = "Institute for Automation of Complex Power Systems, RWTH Aachen University" \
	org.label-schema.author.name = "Steffen Vogel" \
	org.label-schema.author.email = "stvogel@eonerc.rwth-aachen.de" \
	org.label-schema.url = "http://fein-aachen.org/projects/dpsim/" \
	org.label-schema.vcs-url = "https://git.rwth-aachen.de/acs/core/simulation/DPsim"

ADD https://packages.fein-aachen.org/fedora/fein.repo /etc/yum.repos.d/

COPY --from=builder /dpsim/build/*.rpm /tmp
RUN dnf -y install /tmp/*.rpm

ADD requirements.txt .
RUN pip3 install -r requirements.txt

CMD python3
